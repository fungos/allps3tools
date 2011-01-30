
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

#include <mm.h>
#include <gelic.h>
#include <memcpy.h>
#include <beep.h>
#include <dump_lv2.h>

int dump_lv2(u64 start_addr, u64 size)
{
	u64 addr;
	u8 buf[VLAN_ETH_DATA_LEN];
 	int i, result;

	for (addr = start_addr; addr < start_addr + size; addr += 1024)
	{
		memcpy(buf, (u64 *) addr, 1024);

		result = gelic_xmit(gelic_bcast_mac_addr, 0xBEEF, buf, 1024);
		if (result != 0)
			return result;

		for (i = 0; i < 100000; i++)
			__asm__ __volatile__ ("nop");
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
