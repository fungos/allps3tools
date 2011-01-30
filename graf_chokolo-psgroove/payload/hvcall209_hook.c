
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

#include <inttypes.h>
#include <mm.h>
#include <if_vlan.h>
#include <gelic.h>
#include <param.h>
#include <get_new_toc.h>
#include <hvcall209_hook_stage1.h>
#include <hvcall209_hook.h>

#define HVCALL209_HOOK_OFFSET			0x000002E00ULL
#define HVCALL209_INSTR_OFFSET			0x0000345C0ULL

static u32 hook[] =
{
	0xF821FFC1,		/* stdu %r1, -64(%r1) */
	0x7C0802A6,		/* mflr %r0 */
	0xF8010050,		/* std %r0, 80(%r1) */
	0xFBE10038,		/* std %r31, 56(%r1) */
	0x3BE00001,		/* li %r31, 1 */
	0x7BFFF806,		/* rldicr %r31, %r31, 63, 0 */
	0x67FF0000,		/* oris %r31, %r31, <high half word> */
	0x63FF0000,		/* ori %r31, %r31, <low half word> */
	0x7FE903A6, 	/* mtctr %r31 */
	0x4E800421,		/* bctrl */
	0xEBE10038,		/* ld %r31, 56(%r1) */
	0xE8010050,		/* ld %r0, 80(%r1) */
	0x38210040,		/* addi %r1, %r1, 64 */
	0x7C0803A6,		/* mtlr %r0 */
	0x4E800020,		/* blr */
};

int hvcall209_hook(void)
{
	u32 *instr_addr;
	u64 func_addr;
	int i;

	func_addr = (u64) hvcall209_hook_stage1;

	hook[6] |= ((u32) func_addr) >> 16;
	hook[7] |= ((u32) func_addr) & 0xFFFFUL;

	MM_LOAD_BASE(instr_addr, HVCALL209_HOOK_OFFSET);

	memcpy(instr_addr, hook, sizeof(hook));

	MM_LOAD_BASE(instr_addr, HVCALL209_INSTR_OFFSET);

	*instr_addr = (0x4B << 24) | (((s32) HVCALL209_HOOK_OFFSET - (s32) instr_addr) & 0xFFFFFFUL) | 0x1;

	return 0;
}

void hvcall209_hook_stage2(u64 *sp)
{
	u8 buf[64], *sce_module, *arg1, *arg2;
	u64 arg1_size, arg2_size;
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	sce_module = (u8 *) sp[2];
	arg1 = (u8 *) sp[3];
	arg1_size = sp[4];
	arg2 = (u8 *) sp[5];
	arg2_size = sp[6];

	memset(buf, 0, sizeof(buf));
	memcpy(buf, &sce_module, 8);
	memcpy(buf + 8, &arg1, 8);
	memcpy(buf + 16, &arg1_size, 8);
	memcpy(buf + 24, &arg2, 8);
	memcpy(buf + 32, &arg2_size, 8);

	gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 0, buf, sizeof(buf));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 1, sce_module, 0x400);

	if (arg1_size > 0)
		gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 2, arg1, arg1_size);

	if (arg2_size > 0)
		gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 3, arg2, arg2_size);
}
