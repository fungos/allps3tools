
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
#include <dump_lv1.h>

#define HV_BASE							0x8000000014000000ULL
#define HV_OFFSET						0x14000000ULL

static volatile u64 hv_size = 16 * 1024 * 1024;

int dump_lv1(void)
{
	u64 mmap_lpar_addr;
	u8 *hv;
 	int result;

	result = lv1_undocumented_function_114(0x0000000000000000ULL, PAGE_SIZE_4KB, hv_size,
		&mmap_lpar_addr);
	if (result != 0)
		return result;

	MM_LOAD_BASE(hv, HV_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) hv), mmap_lpar_addr,
		hv_size, PAGE_SIZE_4KB, 0x0, 0x0);
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, hv, hv_size);
	if (result != 0)
		return result;

	result = lv1_undocumented_function_115(mmap_lpar_addr);
	if (result != 0)
		return result;

	beep(BEEP_DOUBLE);

	lv1_panic(1);

	return 0;
}
