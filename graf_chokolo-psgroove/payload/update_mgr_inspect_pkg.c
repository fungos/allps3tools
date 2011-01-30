
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
#include <repo_node.h>
#include <beep.h>
#include <dispmgr.h>
#include <ss.h>
#include <update_pkg.h>
#include <update_mgr_inspect_pkg.h>

#define INSPECT_BASE					0x8000000014000000ULL
#define INSPECT_OFFSET					0x14000000ULL
#define INSPECT_PAGE_SIZE				16
#define INSPECT_SIZE					0x1000000
#define INSPECT_VUART_SIZE				(1 << INSPECT_PAGE_SIZE)
#define INSPECT_PKG_SIZE				(INSPECT_SIZE - INSPECT_VUART_SIZE)

static volatile u64 inspect_lpar_addr = 0x700020000000ULL + 0xE900000ULL - INSPECT_SIZE;

static volatile u64 subject_id[2] = { 0x1070000002000001, 0x10700003FF000001 };

int update_mgr_inspect_pkg(void)
{
	u64 pkg_lpar_addr, nread, nwritten, val;
	u64 reques_id, entries[7];
	u8 *msgbuf, *pkgbuf;
	struct dispmgr_header *dispmgr_header;
	struct ss_header *ss_header;
	struct ss_update_mgr_inspect_pkg *ss_update_mgr_inspect_pkg;
	int i, pkg_size, result;

	MM_LOAD_BASE(msgbuf, INSPECT_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) msgbuf), inspect_lpar_addr,
		INSPECT_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	pkg_lpar_addr = inspect_lpar_addr + INSPECT_VUART_SIZE;
	pkgbuf = msgbuf + INSPECT_VUART_SIZE;

	pkg_size = gelic_recv_data(pkgbuf, INSPECT_PKG_SIZE);
	if (pkg_size <= 0)
		return pkg_size;

	memset(msgbuf, 0, INSPECT_VUART_SIZE);

	dispmgr_header = (struct dispmgr_header *) msgbuf;
	dispmgr_header->request_id = 1;
	dispmgr_header->function_id = 0x6000;
	dispmgr_header->request_size = 0x28;
	dispmgr_header->response_size = 0x100;

	ss_header = (struct ss_header *) (dispmgr_header + 1);
	ss_header->packet_id = 0x6002;
	ss_header->function_id = 0x6000;
	ss_header->laid = subject_id[0];
	ss_header->paid = subject_id[1];

	ss_update_mgr_inspect_pkg =
		(struct ss_update_mgr_inspect_pkg *) (ss_header + 1);
	ss_update_mgr_inspect_pkg->field0 = 1;
	ss_update_mgr_inspect_pkg->pkg_type = UPDATE_PKG_TYPE_RL_FOR_PRG;
	ss_update_mgr_inspect_pkg->flags = 0x9;
	ss_update_mgr_inspect_pkg->lpar_id = 2;
	ss_update_mgr_inspect_pkg->pkg_size = 1;
	*(u64 *) ss_update_mgr_inspect_pkg->pkg_data = pkg_lpar_addr;
	*(u64 *) (ss_update_mgr_inspect_pkg->pkg_data + 8) = pkg_size;

	dispmgr_header->request_size += sizeof(struct ss_update_mgr_inspect_pkg) + 0x20;

	result = lv1_write_virtual_uart(DISPMGR_VUART_PORT, inspect_lpar_addr,
		sizeof(struct dispmgr_header) + dispmgr_header->request_size, &nwritten);
	if (result < 0)
		return result;

	result = vuart_wait_for_rx_data(DISPMGR_VUART_PORT);
	if (result < 0)
		return result;

	for (;;)
	{
		result = lv1_read_virtual_uart(DISPMGR_VUART_PORT, inspect_lpar_addr,
			INSPECT_VUART_SIZE, &nread);
		if (result != 0)
			return result;

		if (nread > 0)
			break;
	}

	reques_id = *(u64 *) (ss_update_mgr_inspect_pkg->pkg_data + 0x18);

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, msgbuf, nread);
	if (result < 0)
		return result;

	for (;;)
	{
		entries[0] = lv1_get_repository_node_value(REPO_NODE_LPAR_ID_PME,
			repo_node_key_ss_inspect_request[0], repo_node_key_ss_inspect_request[1],
			repo_node_key_ss_inspect_request[2], reques_id,
			&entries[5], &entries[6]);

		memcpy(&entries[1], repo_node_key_ss_inspect_request, 4 * 8);

		result = gelic_xmit(gelic_bcast_mac_addr, 0xCAFE, entries, sizeof(entries));
		if (result < 0)
			return result;

		if ((entries[5] >> 32) == 3)
			break;

		lv1_pause(0);
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
