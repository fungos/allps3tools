
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
#include <dump_lv1.h>

#define FLASH_OFFSET						0x000C0000ULL
#define FILE_OFFSET						0x00600000ULL

#define HV_BASE							0x8000000014000000ULL
#define HV_OFFSET						0x14000000ULL

#define SYSMGR_PS3_OFFSET					0x0075BE60ULL

static volatile u64 file_offset = FILE_OFFSET - FLASH_OFFSET;
/*
static volatile u64 file_size = 0x00171B88ULL;
*/
static volatile u64 file_size = 0x00161E60ULL;
static volatile char file_name[0x20] = "myfile.self";

static volatile u64 hv_size = 16 * 1024 * 1024;

static volatile char sysmgr_ps3_file_name[0x20] = "/flh/os/myfile.self";

int replace_lv2(void)
{
	u64 start_sector, mmap_lpar_addr;
	u8 buf[3 * STOR_FLASH_BLOCK_SIZE], *hv;
 	int result;

	/*
	result = stor_init(STOR_FLASH_DEV_ID);
	if (result != 0)
		return result;

	start_sector = FLASH_OFFSET / STOR_FLASH_BLOCK_SIZE;

	result = stor_read(0, start_sector, 3, 0x2, buf);
	if (result != 0)
		goto done;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xF00D, buf, sizeof(buf));
	if (result != 0)
		return result;

	*(u32 *) (buf + 0x14) = 0x18;

	*(u64 *) (buf + 0x20) = 0x490;
	*(u64 *) (buf + 0x28) = 0x40000 - 0x30;

	*(u64 *) (buf + 0x470) = file_offset - 0x10;
	*(u64 *) (buf + 0x478) = file_size;
	memcpy(buf + 0x480, file_name, sizeof(file_name));

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xF00D, buf, sizeof(buf));
	if (result != 0)
		return result;

	result = stor_write(0, start_sector, 3, 0x2, buf);
	if (result != 0)
		goto done;
	*/

	result = lv1_undocumented_function_114(0x0000000000000000ULL, PAGE_SIZE_4KB, hv_size,
		&mmap_lpar_addr);
	if (result != 0)
		return result;

	MM_LOAD_BASE(hv, HV_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) hv), mmap_lpar_addr,
		hv_size, PAGE_SIZE_4KB, 0x0, 0x0);
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, hv + SYSMGR_PS3_OFFSET, 0x200);
	if (result != 0)
		return result;

	memcpy(hv + SYSMGR_PS3_OFFSET, sysmgr_ps3_file_name, sizeof(sysmgr_ps3_file_name));

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, hv + SYSMGR_PS3_OFFSET, 0x200);
	if (result != 0)
		return result;

	result = lv1_undocumented_function_115(mmap_lpar_addr);
	if (result != 0)
		return result;

done:

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
