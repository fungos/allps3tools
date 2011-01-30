
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
#include <self_decrypter_hook_stage1.h>
#include <self_decrypter_hook.h>

#define HOOK_OFFSET					0x000002E00ULL
#define INSTR_OFFSET				0x00004FA0CULL
#define FUNC_OFFSET					0x00004E884ULL

struct spu_args
{
	u64 paid;
	u8 *sce_hdr;
	struct appldr_args **appldr_args;
	u64 segment;
	u8 *dst;
	u8 *field28;
	u64 field30;
	u64 field38;
	u64 field40[4];
	u8 field60[16];
	u8 field70[128];
};

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
	0x48000000,		/* b <self decrypter func> */
};

int self_decrypter_hook(void)
{
	u32 *instr_addr;
	u64 func_addr;

	func_addr = (u64) self_decrypter_hook_stage1;

	hook[6] |= ((u32) func_addr) >> 16;
	hook[7] |= ((u32) func_addr) & 0xFFFFUL;
	hook[14] |= ((s32) FUNC_OFFSET - (s32) (HOOK_OFFSET + 14 * 4)) & 0xFFFFFFUL;

	MM_LOAD_BASE(instr_addr, HOOK_OFFSET);

	memcpy(instr_addr, hook, sizeof(hook));

	MM_LOAD_BASE(instr_addr, INSTR_OFFSET);

	*instr_addr = (0x4B << 24) | (((s32) HOOK_OFFSET - (s32) INSTR_OFFSET) & 0xFFFFFFUL) | 0x1;

	return 0;
}

void self_decrypter_hook_stage2(u64 *sp)
{
	static volatile u32 npd_magic = 0x4E504400; /* "NPD\0" */
	u64 new_toc;
	struct spu_args *spu_args;
	u8 *npd;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	spu_args = (struct spu_args *) sp[3];

	npd = spu_args->sce_hdr + *(u64 *) (spu_args->sce_hdr + 0x58) + 0x80;

	if (*(u32 *) npd != npd_magic)
		return;

	gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 0, spu_args, sizeof(struct spu_args));
	gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 1, spu_args->field28, 128);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 2, spu_args->sce_hdr,
		*(u64 *) (spu_args->sce_hdr + 0x10));
}
