
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
#include <patch_dispmgr.h>

#define DISPMGR_BASE						0x80000000133D0000ULL
#define DISPMGR_OFFSET						0x133D0000ULL

static volatile u64 dispmgr_ra_addr = 0x000000000016F000ULL;

static struct
{
	u16 offset;
	u32 opcode;
} dispmgr_patch_table[] =
{
	/*
	{ 0x6B0, 0x60000000 },
	{ 0x74C, 0x3BE00001 },
	{ 0x754, 0x38600000 },
	*/
	{ 0x3BC, 0x60000000 },
	{ 0x458, 0x3BE00001 },
	{ 0x460, 0x38600000 },
};

int patch_dispmgr(void)
{
#define N(a)	(sizeof((a)) / sizeof((a)[0]))

	u64 mmap_lpar_addr;
	u8 *dispmgr;
 	int i, result;

	result = lv1_undocumented_function_114(dispmgr_ra_addr, PAGE_SIZE_4KB, (1ULL << PAGE_SIZE_4KB),
		&mmap_lpar_addr);
	if (result != 0)
		return result;

	MM_LOAD_BASE(dispmgr, DISPMGR_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) dispmgr), mmap_lpar_addr,
		(1ULL << PAGE_SIZE_4KB), PAGE_SIZE_4KB, 0x0, 0x0);
	if (result != 0)
		return result;

	for (i = 0; i < N(dispmgr_patch_table); i++)
		*(u32 *) (dispmgr + dispmgr_patch_table[i].offset) = dispmgr_patch_table[i].opcode;

	result = lv1_undocumented_function_115(mmap_lpar_addr);
	if (result != 0)
		return result;

	return 0;

#undef N
}
