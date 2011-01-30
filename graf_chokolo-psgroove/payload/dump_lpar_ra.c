
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
#include <if_vlan.h>
#include <beep.h>
#include <dump_lpar_ra.h>

int dump_lpar_ra(void)
{
#define N(_a)	(sizeof((_a)) / sizeof((_a)[0]))

	struct
	{
		u64 lpar_addr;
		u64 size;
	} lpar_mem_regions[] =
	{
		{ 0x0000000000000000ULL, 0x1000000ULL },
		{ 0x500000300000ULL, 0xA0000ULL },
		{ 0x700020000000ULL, 0xE900000ULL },
	};
	u64 lpar_addr, ra;
	u8 buf[VLAN_ETH_DATA_LEN];
	int i, j, result;

	for (i = 0; i < N(lpar_mem_regions); i++)
	{
		lpar_addr = lpar_mem_regions[i].lpar_addr;

		memset(buf, 0, sizeof(buf));

		j = 0;

		while (lpar_addr < (lpar_mem_regions[i].lpar_addr + lpar_mem_regions[i].size))
		{
			result = mm_lpar_addr_to_ra(lpar_addr, &ra);
			if (result != 0)
				return result;

			memcpy(buf + j * 16, &lpar_addr, 8);
			memcpy(buf + j * 16 + 8, &ra, 8);

			lpar_addr += (1 << 12);

			j++;
			if (j >= (VLAN_ETH_DATA_LEN / 16))
			{
				result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, buf, j * 16);
				if (result != 0)
					return result;

				memset(buf, 0, sizeof(buf));

				j = 0;
			}
		}

		if (j > 0)
		{
			result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, buf, j * 16);
			if (result != 0)
				return result;
		}
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;

#undef N
}
