
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

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

typedef struct
{
	volatile unsigned int value;
} spinlock_t;

static __inline__ unsigned int spinlock_trylock(spinlock_t *spinlock)
{
	unsigned int new_value, old_value;

	new_value = 1;

	__asm__ __volatile__ (
		"1:\n\t"
		"	lwarx %0, %%r0, %1\n\t"
		"	stwcx. %2, %%r0, %1\n\t"
		"	bne- 1b\n\t"
		"	isync": "=r"(old_value) : "r"(&spinlock->value), "r"(new_value) : "cr0", "memory");

	return old_value;
}

static __inline__ void spinlock_lock(spinlock_t *spinlock)
{
	while (1)
	{
		if (spinlock_trylock(spinlock) == 0)
			break;

		__asm__ __volatile__ ("cctpl");

		while (spinlock->value != 0)
			;

		__asm__ __volatile__ ("cctpm");
	}
}

static __inline__ void spinlock_unlock(spinlock_t *spinlock)
{
	__asm__ __volatile__ ("isync");

	spinlock->value = 0;
}

#endif
