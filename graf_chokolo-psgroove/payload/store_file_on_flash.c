
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
#include <stor.h>
#include <beep.h>
#include <store_file_on_flash.h>

#define FILE_BASE						0x8000000014000000ULL
#define FILE_OFFSET						0x14000000ULL
#define FILE_SIZE						0x001B0000UL

#define FLASH_OFFSET						0x00600000UL

static volatile u64 file_lpar_addr = 0x700020000000ULL + 0xE900000ULL - FILE_SIZE;

int store_file_on_flash(void)
{
	u8 *file;
	u64 start_sector;
 	int file_size, i, result;

	result = stor_init(STOR_FLASH_DEV_ID);
	if (result != 0)
		return result;

	MM_LOAD_BASE(file, FILE_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) file), file_lpar_addr,
		FILE_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	file_size = gelic_recv_data(file, FILE_SIZE);
	if (file_size <= 0)
		return file_size;

	for (i = 0; i < file_size / (8 * STOR_FLASH_BLOCK_SIZE); i++)
	{
		start_sector = FLASH_OFFSET / STOR_FLASH_BLOCK_SIZE + i * 8;

		result = gelic_xmit(gelic_bcast_mac_addr, 0xBEEF, &start_sector, 8);
		if (result != 0)
			goto done;

		result = stor_write(0, start_sector, 8, 0x2, file + i * 8 * STOR_FLASH_BLOCK_SIZE);
		if (result != 0)
			goto done;
	}

	i *= 8;

	for (; i < (file_size + STOR_FLASH_BLOCK_SIZE - 1) / STOR_FLASH_BLOCK_SIZE; i++)
	{
		start_sector = FLASH_OFFSET / STOR_FLASH_BLOCK_SIZE + i;

		result = gelic_xmit(gelic_bcast_mac_addr, 0xBEEF, &start_sector, 8);
		if (result != 0)
			goto done;

		result = stor_write(0, start_sector, 1, 0x2, file + i * STOR_FLASH_BLOCK_SIZE);
		if (result != 0)
			goto done;
	}

	beep(BEEP_DOUBLE);

done:

	lv1_panic(1);

	return 0;
}
