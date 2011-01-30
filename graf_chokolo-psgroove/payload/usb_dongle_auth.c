
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <lv1call.h>
#include <mm.h>
#include <gelic.h>
#include <beep.h>
#include <memset.h>
#include <memcpy.h>
#include <vuart.h>
#include <dispmgr.h>
#include <ss.h>
#include <sha1.h>
#include <usb_dongle_auth.h>

#define USB_DONGLE_AUTH_BASE				0x80000000133A0000ULL
#define USB_DONGLE_AUTH_OFFSET				0x133A0000ULL
#define USB_DONGLE_AUTH_PAGE_SIZE			12
#define USB_DONGLE_AUTH_SIZE				(1 << USB_DONGLE_AUTH_PAGE_SIZE)

static volatile u64 usb_dongle_auth_subject_id[2] = { 0x1070000002000001, 0x1070000044000001 };

static volatile u64 update_mgr_subject_id[2] = { 0x1070000002000001, 0x10700003FF000001 };

static u8 master_key[20] =
{
	0x46, 0xDC, 0xEA, 0xD3, 0x17, 0xFE, 0x45, 0xD8, 0x09, 0x23,
	0xEB, 0x97, 0xE4, 0x95, 0x64, 0x10, 0xD4, 0xCD, 0xB2, 0xC2,
};

/*
static u8 dongle_key_0xAAAA[20] =
{
	0x04, 0x4E, 0x61, 0x1B, 0xA6, 0xA6, 0xE3, 0x9A, 0x98, 0xCF,
	0x35, 0x81, 0x2C, 0x80, 0x68, 0xC7, 0xFC, 0x5F, 0x7A, 0xE8,
};
*/

int usb_dongle_auth(void)
{
	u64 vuart_lpar_addr, muid, nread, nwritten;
	u8 *msgbuf;
	struct dispmgr_header *dispmgr_header;
	struct ss_header *ss_header;
	struct ss_usb_dongle_auth_generate_challenge *ss_usb_dongle_auth_generate_challenge;
	struct ss_usb_dongle_auth_verify_response *ss_usb_dongle_auth_verify_response;
	struct ss_update_mgr_read_eprom *ss_update_mgr_read_eprom;
	u16 dongle_id;
	u8 dongle_key[20], challenge[20], response[20];
	int result;

	result = lv1_allocate_memory(USB_DONGLE_AUTH_SIZE, USB_DONGLE_AUTH_PAGE_SIZE,
		0, 0, &vuart_lpar_addr, &muid);
 	if (result != 0)
 		return result;

	MM_LOAD_BASE(msgbuf, USB_DONGLE_AUTH_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) msgbuf), vuart_lpar_addr,
		USB_DONGLE_AUTH_SIZE, USB_DONGLE_AUTH_PAGE_SIZE, 0, 0);
 	if (result != 0)
 		return result;

	memset(msgbuf, 0, USB_DONGLE_AUTH_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 1;
	dispmgr_header->function_id = 0x24000;
	dispmgr_header->request_size = sizeof(struct ss_header);
	dispmgr_header->response_size = sizeof(struct ss_header) + 0x17;

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	memset(ss_header, 0, sizeof(struct ss_header));
	ss_header->packet_id = 0x24001;
	ss_header->function_id = 0x24000;
	ss_header->laid = usb_dongle_auth_subject_id[0];
	ss_header->paid = usb_dongle_auth_subject_id[1];

	ss_usb_dongle_auth_generate_challenge =
		(struct ss_usb_dongle_auth_generate_challenge *) (ss_header + 1);
	memset(ss_usb_dongle_auth_generate_challenge, 0,
		sizeof(struct ss_usb_dongle_auth_generate_challenge));

	dispmgr_header->request_size += sizeof(struct ss_usb_dongle_auth_generate_challenge);

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
		sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
	if (result < 0)
		return result;

	result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
	if (result < 0)
		return result;

	result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr, USB_DONGLE_AUTH_SIZE,
		&nread);
	if (result < 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, msgbuf, nread);
	if (result < 0)
		return result;

	memcpy(challenge, msgbuf + sizeof(dispmgr_header) + sizeof(struct ss_header) + 3, 20);

	dongle_id = 0x5555;

	hmac_sha1(master_key, 20, &dongle_id, 2, dongle_key);

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, dongle_key, 20);
	if (result < 0)
		return result;

	hmac_sha1(dongle_key, 20, challenge, 20, response);

	memset(msgbuf, 0, USB_DONGLE_AUTH_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 2;
	dispmgr_header->function_id = 0x24000;
	dispmgr_header->request_size = sizeof(struct ss_header);
	dispmgr_header->response_size = sizeof(struct ss_header) + sizeof(struct ss_usb_dongle_auth_verify_response);

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	memset(ss_header, 0, sizeof(struct ss_header));
	ss_header->packet_id = 0x24002;
	ss_header->function_id = 0x24000;
	ss_header->laid = usb_dongle_auth_subject_id[0];
	ss_header->paid = usb_dongle_auth_subject_id[1];

	ss_usb_dongle_auth_verify_response =
		(struct ss_usb_dongle_auth_verify_response *) (ss_header + 1);
	memset(ss_usb_dongle_auth_verify_response, 0,
		sizeof(struct ss_usb_dongle_auth_verify_response));
	ss_usb_dongle_auth_verify_response->header[0] = 0x2E;
	ss_usb_dongle_auth_verify_response->header[1] = 0x2;
	ss_usb_dongle_auth_verify_response->header[2] = 0x2;
	ss_usb_dongle_auth_verify_response->dongle_id = dongle_id;
	memcpy(ss_usb_dongle_auth_verify_response->response, response, 20);

	dispmgr_header->request_size += sizeof(struct ss_usb_dongle_auth_verify_response);

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF,
		msgbuf, sizeof(struct dispmgr_header) + dispmgr_header->request_size);
	if (result < 0)
		return result;

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
		sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
	if (result < 0)
		return result;

	result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
	if (result < 0)
		return result;

	result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr, USB_DONGLE_AUTH_SIZE,
		&nread);
	if (result < 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, msgbuf, nread);
	if (result < 0)
		return result;

	memset(msgbuf, 0, USB_DONGLE_AUTH_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 3;
	dispmgr_header->function_id = 0x6000;
	dispmgr_header->request_size = sizeof(struct ss_header);
	dispmgr_header->response_size = sizeof(struct ss_header) + sizeof(struct ss_update_mgr_read_eprom);

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	memset(ss_header, 0, sizeof(struct ss_header));
	ss_header->packet_id = 0x600B;
	ss_header->function_id = 0x6000;
	ss_header->laid = update_mgr_subject_id[0];
	ss_header->paid = update_mgr_subject_id[1];

	ss_update_mgr_read_eprom =
		(struct ss_update_mgr_read_eprom *) (ss_header + 1);
	memset(ss_update_mgr_read_eprom, 0,
		sizeof(struct ss_update_mgr_read_eprom));
	ss_update_mgr_read_eprom->offset = 0x48C07;

	dispmgr_header->request_size += sizeof(struct ss_update_mgr_read_eprom);

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
		sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
	if (result < 0)
		return result;

	result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
	if (result < 0)
		return result;

	result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr, USB_DONGLE_AUTH_SIZE,
		&nread);
	if (result < 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, msgbuf, nread);
	if (result < 0)
		return result;

	beep(BEEP_DOUBLE);

	//lv1_panic(1);

	return 0;
}
