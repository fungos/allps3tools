
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
#include <memset.h>
#include <vuart.h>
#include <dispmgr.h>
#include <ss.h>
#include <update_mgr_set_token.h>

#define UPDATE_MGR_SET_TOKEN_BASE				0x80000000133A0000ULL
#define UPDATE_MGR_SET_TOKEN_OFFSET				0x133A0000ULL
#define UPDATE_MGR_SET_TOKEN_PAGE_SIZE			12
#define UPDATE_MGR_SET_TOKEN_SIZE				(1 << UPDATE_MGR_SET_TOKEN_PAGE_SIZE)

static volatile u64 subject_id[2] = { 0x1070000002000001, 0x10700003FF000001 };

/*
static u8 token[0x50] =
{
	0xF6, 0x58, 0xDB, 0xAC, 0x63, 0xEB, 0x47, 0x99, 0xE2, 0x63,
	0xC0, 0x10, 0x66, 0x42, 0x3D, 0xF7, 0x34, 0x29, 0x90, 0x61,
	0x23, 0xED, 0x89, 0xEC, 0x21, 0x9E, 0xE2, 0x8B, 0x83, 0xF9,
	0x87, 0x2F, 0x32, 0x50, 0xEC, 0xC3, 0xD0, 0x3D, 0xEA, 0x6E,
	0x14, 0xE0, 0x81, 0xA2, 0x67, 0xCE, 0x86, 0xF7, 0x7A, 0xFE,
	0xDF, 0x11, 0xAB, 0x39, 0xE1, 0xCE, 0x57, 0x06, 0x42, 0xC0,
	0x2B, 0xB2, 0x3F, 0x49, 0x04, 0xC7, 0xE7, 0x58, 0x70, 0x19,
	0x6A, 0xF1, 0xE4, 0x94, 0x32, 0x36, 0x61, 0xB0, 0xA6, 0xB5,
};
*/

static u8 token[0x50] =
{
	0xB8, 0x26, 0x7B, 0x29, 0x21, 0x2A, 0xD6, 0x7F, 0x49, 0xFA,
	0x48, 0x2F, 0xB2, 0xC2, 0xBB, 0x36, 0xE6, 0x91, 0x59, 0x28,
	0xF9, 0x1E, 0x99, 0x8A, 0x6B, 0x9B, 0xEC, 0xA0, 0x8A, 0x2E,
	0xB5, 0x8F, 0xCE, 0x67, 0x14, 0x01, 0xA0, 0x95, 0xD6, 0x10,
	0x02, 0x1F, 0x94, 0x98, 0xDB, 0x9D, 0x1C, 0x0B, 0x8A, 0x8F,
	0x4A, 0x71, 0xC5, 0x3A, 0x5C, 0xB1, 0xC4, 0x21, 0x2C, 0x50,
	0xFE, 0xA2, 0x63, 0x13, 0x8A, 0x01, 0x59, 0x69, 0xA0, 0x15,
	0xE1, 0xD2, 0x41, 0x21, 0xBB, 0x41, 0x46, 0x9C, 0xD7, 0xAD,
};

int update_mgr_set_token(void)
{
	u64 vuart_lpar_addr, muid, nread, nwritten, val;
	u8 *msgbuf;
	struct dispmgr_header *dispmgr_header;
	struct ss_header *ss_header;
	struct ss_update_mgr_set_token *ss_update_mgr_set_token;
	int i, result;

	result = lv1_allocate_memory(UPDATE_MGR_SET_TOKEN_SIZE, UPDATE_MGR_SET_TOKEN_PAGE_SIZE,
		0, 0, &vuart_lpar_addr, &muid);
 	if (result != 0)
 		return result;

	MM_LOAD_BASE(msgbuf, UPDATE_MGR_SET_TOKEN_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) msgbuf), vuart_lpar_addr,
		UPDATE_MGR_SET_TOKEN_SIZE, UPDATE_MGR_SET_TOKEN_PAGE_SIZE, 0, 0);
	if (result != 0)
		return result;

	memset(msgbuf, 0, UPDATE_MGR_SET_TOKEN_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 1;
	dispmgr_header->function_id = 0x6000;
	dispmgr_header->request_size = 0x28;
	dispmgr_header->response_size = 0x100;

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	memset(ss_header, 0, sizeof(struct ss_header));
	ss_header->packet_id = 0x600A;
	ss_header->function_id = 0x6000;
	ss_header->laid = subject_id[0];
	ss_header->paid = subject_id[1];

	ss_update_mgr_set_token =
		(struct ss_update_mgr_set_token *) (ss_header + 1);
	memset(ss_update_mgr_set_token, 0,
		sizeof(struct ss_update_mgr_set_token));
	ss_update_mgr_set_token->token_size = 0x50;
	memcpy(ss_update_mgr_set_token->token, token, 0x50);

	dispmgr_header->request_size += sizeof(struct ss_update_mgr_set_token);

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
		sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
	if (result < 0)
		return result;

	for (;;)
	{
		result = lv1_get_virtual_uart_param(DISPMGR_VUART_PORT, VUART_PARAM_RX_BYTES, &val);
		if (result < 0)
			return result;

		if (val != 0)
			break;
	}

	for (;;)
	{
		result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
			UPDATE_MGR_SET_TOKEN_SIZE, &nread);
		if (result < 0)
			return result;

		if (nread == 0)
			break;

		result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, msgbuf, nread);
		if (result < 0)
			return result;
	}

	lv1_panic(1);

	return 0;
}
