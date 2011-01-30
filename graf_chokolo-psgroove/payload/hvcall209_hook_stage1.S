
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

.globl hvcall209_hook_stage1
hvcall209_hook_stage1:

	stdu %r1, -0x200(%r1)
	mflr %r0
	std %r0, 0x210(%r1)
	std %r31, 0x1F8(%r1)
	std %r30, 0x1F0(%r1)
	std %r29, 0x1E8(%r1)
	std %r28, 0x1E0(%r1)
	std %r27, 0x1D8(%r1)
	std %r26, 0x1D0(%r1)
	std %r25, 0x1C8(%r1)
	std %r24, 0x1C0(%r1)
	std %r23, 0x1B8(%r1)
	std %r22, 0x1B0(%r1)
	std %r21, 0x1A8(%r1)
	std %r20, 0x1A0(%r1)
	std %r19, 0x198(%r1)
	std %r18, 0x190(%r1)
	std %r17, 0x188(%r1)
	std %r16, 0x180(%r1)
	std %r15, 0x178(%r1)
	std %r14, 0x170(%r1)
	std %r13, 0x168(%r1)
	std %r12, 0x160(%r1)
	std %r11, 0x158(%r1)
	std %r10, 0x150(%r1)
	std %r9, 0x148(%r1)
	std %r8, 0x140(%r1)
	std %r7, 0x138(%r1)
	std %r6, 0x130(%r1)
	std %r5, 0x128(%r1)
	std %r4, 0x120(%r1)
	std %r3, 0x118(%r1)
	std %r2, 0x110(%r1)

	sc 1

	std %r5, 0x108(%r1)
	std %r4, 0x100(%r1)
	std %r3, 0xF8(%r1)

	addi %r3, %r1, 0x118
	bl hvcall209_hook_stage2

	ld %r31, 0x1F8(%r1)
	ld %r30, 0x1F0(%r1)
	ld %r29, 0x1E8(%r1)
	ld %r28, 0x1E0(%r1)
	ld %r27, 0x1D8(%r1)
	ld %r26, 0x1D0(%r1)
	ld %r25, 0x1C8(%r1)
	ld %r24, 0x1C0(%r1)
	ld %r23, 0x1B8(%r1)
	ld %r22, 0x1B0(%r1)
	ld %r21, 0x1A8(%r1)
	ld %r20, 0x1A0(%r1)
	ld %r19, 0x198(%r1)
	ld %r18, 0x190(%r1)
	ld %r17, 0x188(%r1)
	ld %r16, 0x180(%r1)
	ld %r15, 0x178(%r1)
	ld %r14, 0x170(%r1)
	ld %r13, 0x168(%r1)
	ld %r12, 0x160(%r1)
	ld %r11, 0x158(%r1)
	ld %r10, 0x150(%r1)
	ld %r9, 0x148(%r1)
	ld %r8, 0x140(%r1)
	ld %r7, 0x138(%r1)
	ld %r6, 0x130(%r1)

	ld %r5, 0x108(%r1)
	ld %r4, 0x100(%r1)
	ld %r3, 0xF8(%r1)

	ld %r2, 0x110(%r1)
	ld %r0, 0x210(%r1)
	addi %r1, %r1, 0x200
	mtlr %r0
	blr
