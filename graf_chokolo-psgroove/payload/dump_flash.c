
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
#include <stor.h>
#include <memcpy.h>
#include <beep.h>
#include <dump_flash.h>

int dump_flash(void)
{
	u64 region_id, sector;
	u8 buf[1024];
 	int i, result;

	result = stor_init(STOR_FLASH_DEV_ID);
 	if (result != 0)
 		return result;

	for (region_id = 0; region_id < 1; region_id++)
	{
		for (sector = 0; sector < 0x8000; sector++)
		{
			result = stor_read(region_id, sector, 1, 0x2, buf);

			if (result != 0)
			{
				memcpy(buf, &region_id, 8);
				memcpy(buf + 8, &sector, 8);
				memcpy(buf + 16, &result, 4);

				result = gelic_xmit(gelic_bcast_mac_addr, 0xBEEF, buf, 16 + 4);
				if (result != 0)
					return result;
			}
			else
			{
				result = gelic_xmit(gelic_bcast_mac_addr, 0xBEEF, buf, STOR_FLASH_BLOCK_SIZE);
				if (result != 0)
					return result;
			}

			for (i = 0; i < 100000; i++)
				__asm__ __volatile__ ("nop");
		}
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
