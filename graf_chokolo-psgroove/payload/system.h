
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

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define mb()					__asm__ __volatile__ ("sync" : : : "memory")
#define rmb()					__asm__ __volatile__ ("sync" : : : "memory")
#define wmb()					__asm__ __volatile__ ("sync" : : : "memory")
#define lwsync()				__asm__ __volatile__ ("lwsync" : : : "memory")
#define eieio()					__asm__ __volatile__ ("eieio" : : : "memory")

#define hard_irq_disable()		\
	__asm__ __volatile__ ("li %%r0, 2; mtmsrd %%r0, 1" : : : "r0")

#define hard_irq_enable()		\
	__asm__ __volatile__ ("li %%r0, 0; ori %%r0, %%r0, 0x8002; lwsync; mtmsrd %%r0, 1" : : : "r0")

#endif
