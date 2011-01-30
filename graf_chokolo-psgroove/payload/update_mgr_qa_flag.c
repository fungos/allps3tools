
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
#include <update_mgr_qa_flag.h>

#define UPDATE_MGR_QA_FLAG_BASE				0x80000000133A0000ULL
#define UPDATE_MGR_QA_FLAG_OFFSET			0x133A0000ULL
#define UPDATE_MGR_QA_FLAG_PAGE_SIZE		12
#define UPDATE_MGR_QA_FLAG_SIZE				(1 << UPDATE_MGR_QA_FLAG_PAGE_SIZE)

static volatile u64 subject_id[2] = { 0x1070000002000001, 0x10700003FF000001 };

int update_mgr_qa_flag(void)
{
	u64 vuart_lpar_addr, muid, nread, nwritten;
	u8 *msgbuf;
	struct dispmgr_header *dispmgr_header;
	struct ss_header *ss_header;
	struct ss_update_mgr_read_eprom *ss_update_mgr_read_eprom;
	struct ss_update_mgr_write_eprom *ss_update_mgr_write_eprom;
	int result;

	result = lv1_allocate_memory(UPDATE_MGR_QA_FLAG_SIZE, UPDATE_MGR_QA_FLAG_PAGE_SIZE,
		0, 0, &vuart_lpar_addr, &muid);
 	if (result != 0)
 		return result;

	MM_LOAD_BASE(msgbuf, UPDATE_MGR_QA_FLAG_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) msgbuf), vuart_lpar_addr,
		UPDATE_MGR_QA_FLAG_SIZE, UPDATE_MGR_QA_FLAG_PAGE_SIZE, 0, 0);
	if (result != 0)
		return result;

	memset(msgbuf, 0, UPDATE_MGR_QA_FLAG_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 1;
	dispmgr_header->function_id = 0x6000;
	dispmgr_header->request_size = 0x28;
	dispmgr_header->response_size = 0x100;

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	memset(ss_header, 0, sizeof(struct ss_header));
	ss_header->laid = subject_id[0];
	ss_header->paid = subject_id[1];

	/*
	ss_header->packet_id = 0x600B;

	ss_update_mgr_read_eprom =
		(struct ss_update_mgr_read_eprom *) (ss_header + 1);
	memset(ss_update_mgr_read_eprom, 0,
		sizeof(struct ss_update_mgr_read_eprom));
	ss_update_mgr_read_eprom->offset = 0x48C0A;

	dispmgr_header->request_size += sizeof(struct ss_update_mgr_read_eprom);
	*/

	ss_header->packet_id = 0x600C;

	ss_update_mgr_write_eprom =
		(struct ss_update_mgr_write_eprom *) (ss_header + 1);
	memset(ss_update_mgr_write_eprom, 0,
		sizeof(struct ss_update_mgr_write_eprom));
	ss_update_mgr_write_eprom->offset = 0x48C0A;
	ss_update_mgr_write_eprom->value = 0x00;

	dispmgr_header->request_size += sizeof(struct ss_update_mgr_write_eprom);

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
		sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
	if (result < 0)
		return result;

	result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
	if (result < 0)
		return result;

	for (;;)
	{
		result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, vuart_lpar_addr,
			UPDATE_MGR_QA_FLAG_SIZE, &nread);
		if (result < 0)
			return result;

		if (nread == 0)
			break;

		result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, msgbuf, nread);
		if (result < 0)
			return result;
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
