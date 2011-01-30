
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
#include <vuart.h>
#include <dispmgr.h>
#include <ss.h>
#include <sc_mgr_get_region_data.h>

#define SC_MGR_GET_REGION_DATA_BASE				0x80000000133A0000ULL
#define SC_MGR_GET_REGION_DATA_OFFSET			0x133A0000ULL
#define SC_MGR_GET_REGION_DATA_PAGE_SIZE		12
#define SC_MGR_GET_REGION_DATA_SIZE				(1 << SC_MGR_GET_REGION_DATA_PAGE_SIZE)

static volatile u64 subject_id[2] = { 0x1070000002000001, 0x10700003FF000001 };

int sc_mgr_get_region_data(void)
{
	u64 vuart_lpar_addr, muid, nread, nwritten;
	u8 *msgbuf;
	struct dispmgr_header *dispmgr_header;
	struct ss_header *ss_header;
	struct ss_sc_mgr_get_region_data *ss_sc_mgr_get_region_data;
	int i, result;

	result = lv1_allocate_memory(SC_MGR_GET_REGION_DATA_SIZE, SC_MGR_GET_REGION_DATA_PAGE_SIZE,
		0, 0, &vuart_lpar_addr, &muid);
 	if (result != 0)
 		return result;

	MM_LOAD_BASE(msgbuf, SC_MGR_GET_REGION_DATA_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) msgbuf), vuart_lpar_addr,
		SC_MGR_GET_REGION_DATA_SIZE, SC_MGR_GET_REGION_DATA_PAGE_SIZE, 0, 0);
	if (result != 0)
		return result;

	for (i = 0; i < 16; i++)
	{
		memset(msgbuf, 0, SC_MGR_GET_REGION_DATA_SIZE);

		dispmgr_header = (struct dispmgr_header *) msgbuf;
		dispmgr_header->request_id = i + 1;
		dispmgr_header->function_id = 0x9000;
		dispmgr_header->request_size = sizeof(struct ss_header);
		dispmgr_header->response_size = sizeof(struct ss_header) +
			sizeof(struct ss_sc_mgr_get_region_data) + 0x30;

		ss_header = (struct ss_header *) (dispmgr_header + 1);
		memset(ss_header, 0, sizeof(struct ss_header));
		ss_header->packet_id = 0x9006;
		ss_header->function_id = 0x9000;
		ss_header->laid = subject_id[0];
		ss_header->paid = subject_id[1];

		ss_sc_mgr_get_region_data = (struct ss_sc_mgr_get_region_data *) (ss_header + 1);
		memset(ss_sc_mgr_get_region_data, 0, sizeof(struct ss_sc_mgr_get_region_data));
		ss_sc_mgr_get_region_data->id = i;
		ss_sc_mgr_get_region_data->data_size = 0x30;

		dispmgr_header->request_size += sizeof(struct ss_sc_mgr_get_region_data) +
			ss_sc_mgr_get_region_data->data_size;

		result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
			sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
		if (result < 0)
			return result;

		result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
		if (result < 0)
			return result;

		result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
			SC_MGR_GET_REGION_DATA_SIZE, &nread);
		if (result < 0)
			return result;

		result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, msgbuf, nread);
		if (result < 0)
			return result;
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
