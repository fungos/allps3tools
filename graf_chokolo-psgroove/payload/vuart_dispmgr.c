
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
#include <vuart_dispmgr.h>

#define VUART_DISPMGR_BASE				0x80000000133A0000ULL
#define VUART_DISPMGR_OFFSET			0x133A0000ULL
#define VUART_DISPMGR_PAGE_SIZE			12
#define VUART_DISPMGR_SIZE				(1 << VUART_DISPMGR_PAGE_SIZE)

static volatile u64 subject_id[2] = { 0x1070000002000001, 0x10700003FF000001 };

int vuart_dispmgr(void)
{
	u64 vuart_lpar_addr, muid, nread, nwritten;
	void *msgbuf;
	struct dispmgr_header *dispmgr_header;
	struct ss_header *ss_header;
	struct ss_update_mgr_get_package_info *ss_update_mgr_get_package_info;
	int result;

	result = lv1_allocate_memory(VUART_DISPMGR_SIZE, VUART_DISPMGR_PAGE_SIZE,
		0, 0, &vuart_lpar_addr, &muid);
 	if (result != 0)
 		return result;

	MM_LOAD_BASE(msgbuf, VUART_DISPMGR_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) msgbuf), vuart_lpar_addr,
		VUART_DISPMGR_SIZE, VUART_DISPMGR_PAGE_SIZE, 0, 0);
 	if (result != 0)
 		return result;

	memset(msgbuf, 0, VUART_DISPMGR_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 1;
	dispmgr_header->function_id = 0x6000;
	dispmgr_header->request_size = 0x28;
	dispmgr_header->response_size = 0x100;

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	memset(ss_header, 0, sizeof(struct ss_header));
	ss_header->packet_id = 0x6003;
	ss_header->function_id = 0x6000;
	ss_header->laid = subject_id[0];
	ss_header->paid = subject_id[1];

	ss_update_mgr_get_package_info =
		(struct ss_update_mgr_get_package_info *) (ss_header + 1);
	memset(ss_update_mgr_get_package_info, 0,
		sizeof(struct ss_update_mgr_get_package_info));
	ss_update_mgr_get_package_info->type = 1;

	dispmgr_header->request_size += sizeof(struct ss_update_mgr_get_package_info);

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
		sizeof(struct dispmgr_header) + sizeof(struct ss_header) +
		sizeof(struct ss_update_mgr_get_package_info), &nwritten);
	if (result < 0)
		return result;

	result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
	if (result < 0)
		return result;

	for (;;)
	{
		result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr, VUART_DISPMGR_SIZE,
			&nread);
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
