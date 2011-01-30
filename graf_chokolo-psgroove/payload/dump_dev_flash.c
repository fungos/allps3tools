
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

#include <gelic.h>
#include <stor.h>
#include <beep.h>
#include <memcpy.h>
#include <dump_dev_flash.h>

int dump_dev_flash(void)
{
	u64 sector;
	u8 buf[1024];
 	int i, result;

	result = stor_init(4);
 	if (result != 0)
 		return result;

	for (sector = 0; sector < 0x70000; sector++)
	{
		result = stor_read(2, sector, 1, 0x4, buf);

		if (result != 0)
		{
			memcpy(buf, &sector, 8);
			memcpy(buf + 8, &result, 4);

			result = gelic_xmit(gelic_bcast_mac_addr, 0xCAFE, buf, 8 + 4);
			if (result != 0)
				return result;
		}
		else
		{
			result = gelic_xmit(gelic_bcast_mac_addr, 0xBEEF, buf, 0x200);
			if (result != 0)
				return result;
		}

		for (i = 0; i < 100000; i++)
			__asm__ __volatile__ ("nop");
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
