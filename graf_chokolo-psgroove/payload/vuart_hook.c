
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
#include <vuart_write_hook_stage1.h>
#include <vuart_read_hook_stage1.h>
#include <vuart_hook.h>

#define VUART_HOOK_OFFSET			0x000002E00ULL

#define VUART_BUFFER_BASE			0x80000000133A0000ULL
#define VUART_BUFFER_OFFSET			0x133A0000ULL

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

static u64 vuart_write_hvcs[] =
{
	0x80000000000145DCULL,
	0x8000000000014814ULL,
	0x8000000000014EECULL,
	0x8000000000015584ULL,
};

static u64 vuart_read_hvcs[] =
{
	0x8000000000014568ULL,
	0x8000000000015128ULL,
};

int vuart_hook(void)
{
#define N(_a)	(sizeof((_a)) / sizeof((_a)[0]))

	u32 *instr_addr;
	u64 func_addr;
	int i;

	for (i = 0; i < N(vuart_write_hvcs); i++)
	{
		func_addr = (u64) vuart_write_hook_stage1;

		hook[6] &= 0xFFFF0000UL;
		hook[6] |= ((u32) func_addr) >> 16;
		hook[7] &= 0xFFFF0000UL;
		hook[7] |= ((u32) func_addr) & 0xFFFFUL;

		MM_LOAD_BASE(instr_addr, VUART_HOOK_OFFSET);

		memcpy(instr_addr, hook, sizeof(hook));

		instr_addr = (u32 *) vuart_write_hvcs[i];

		*instr_addr = (0x4B << 24) | (((s32) VUART_HOOK_OFFSET - (s32) instr_addr) & 0xFFFFFFUL) | 0x1;
	}

	for (i = 0; i < N(vuart_read_hvcs); i++)
	{
		func_addr = (u64) vuart_read_hook_stage1;

		hook[6] &= 0xFFFF0000UL;
		hook[6] |= ((u32) func_addr) >> 16;
		hook[7] &= 0xFFFF0000UL;
		hook[7] |= ((u32) func_addr) & 0xFFFFUL;

		MM_LOAD_BASE(instr_addr, VUART_HOOK_OFFSET + sizeof(hook));

		memcpy(instr_addr, hook, sizeof(hook));

		instr_addr = (u32 *) vuart_read_hvcs[i];

		*instr_addr = (0x4B << 24) | (((s32) (VUART_HOOK_OFFSET + sizeof(hook)) - (s32) instr_addr) & 0xFFFFFFUL) | 0x1;
	}

	return 0;

#undef N
}

void vuart_write_hook_stage2(u64 *sp, int hvc_result, u64 bytes_written)
{
	u64 port, buffer, buffer_size, pte_index;
	u8 *msgbuf, buf[64];
	u64 new_toc;
	int result;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	port = sp[0];
	buffer = sp[1];
	buffer_size = sp[2];

	memset(buf, 0, sizeof(buf));
	memcpy(buf, &port, 8);
	memcpy(buf + 12, &hvc_result, 4);
	memcpy(buf + 16, &buffer, 8);
	memcpy(buf + 24, &buffer_size, 8);
	memcpy(buf + 32, &bytes_written, 8);

	gelic_xmit_data(gelic_bcast_mac_addr, 0xBA00 + port, buf, sizeof(buf));

	if ((hvc_result == 0) && (bytes_written > 0))
	{
		MM_LOAD_BASE(msgbuf, VUART_BUFFER_OFFSET);

		result = mm_insert_htab_entry(0, MM_EA2VA((u64) msgbuf), buffer & ~0xFFFULL,
			0xC, 0, 0x3, &pte_index);
 		if (result != 0)
 			return;

		gelic_xmit_data(gelic_bcast_mac_addr, 0xBA00 + port,
			msgbuf + (buffer & 0xFFF), bytes_written);

		lv1_write_htab_entry(0, pte_index, 0, 0);
	}
}

void vuart_read_hook_stage2(u64 *sp, int hvc_result, u64 bytes_read)
{
	u64 port, buffer, buffer_size, pte_index;
	u8 *msgbuf, buf[64];
	u64 new_toc;
	int result;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	port = sp[0];
	buffer = sp[1];
	buffer_size = sp[2];

	memset(buf, 0, sizeof(buf));
	memcpy(buf, &port, 8);
	memcpy(buf + 12, &hvc_result, 4);
	memcpy(buf + 16, &buffer, 8);
	memcpy(buf + 24, &buffer_size, 8);
	memcpy(buf + 32, &bytes_read, 8);

	gelic_xmit_data(gelic_bcast_mac_addr, 0xBB00 + port, buf, sizeof(buf));

	if ((hvc_result == 0) && (bytes_read > 0))
	{
		MM_LOAD_BASE(msgbuf, VUART_BUFFER_OFFSET);

		result = mm_insert_htab_entry(0, MM_EA2VA((u64) msgbuf), buffer & ~0xFFFULL,
			0xC, 0, 0x3, &pte_index);
 		if (result != 0)
 			return;

		gelic_xmit_data(gelic_bcast_mac_addr, 0xBB00 + port, msgbuf + (buffer & 0xFFF), bytes_read);

		lv1_write_htab_entry(0, pte_index, 0, 0);
	}
}
