
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

.globl get_new_toc
get_new_toc:

	stdu %r1, -0x40(%r1)
	mflr %r0
	std %r0, 0x50(%r1)

	bl 1f

1:

	mflr %r3
	lis %r4, 1b@ha
	addi %r4, %r4, 1b@l
	subf. %r3, %r4, %r3

	lis %r4, __toc_start@ha
	addi %r4, %r4, __toc_start@l
	add %r3, %r4, %r3
	li %r4, 0
	ori %r4, %r4, 0x8000
	add %r3, %r3, %r4

	ld %r0, 0x50(%r1)
	addi %r1, %r1, 0x40
	mtlr %r0
	blr
