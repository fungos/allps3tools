
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
#include <inttypes.h>
#include <mm.h>
#include <gelic.h>

#define PAYLOAD_BASE			0x8000000013390000ULL
#define PAYLOAD_OFFSET			0x13390000ULL
#define PAYLOAD_PAGE_SIZE		16
#define PAYLOAD_SIZE			(1 << PAYLOAD_PAGE_SIZE)

int main(void)
{
	u64 payload_lpar_addr, muid;
	u8 *payload;
	int payload_size, i, result;

	result = mm_init();
	if (result != 0)
		goto error;

	result = gelic_init();
	if (result != 0)
		goto error;

	result = lv1_allocate_memory(PAYLOAD_SIZE, PAYLOAD_PAGE_SIZE, 0, 0,
		&payload_lpar_addr, &muid);
	if (result != 0)
		return result;

	MM_LOAD_BASE(payload, PAYLOAD_OFFSET);

	result = mm_map_lpar_memory_region(payload_lpar_addr, (u64) payload, PAYLOAD_SIZE, 0xC, 0);
	if (result != 0)
		goto error;

	memset(payload, 0, PAYLOAD_SIZE);

	*(u64 *) (payload + PAYLOAD_SIZE - 1 * 8) = payload_lpar_addr;
	*(u64 *) (payload + PAYLOAD_SIZE - 2 * 8) = PAYLOAD_SIZE;
	*(u64 *) (payload + PAYLOAD_SIZE - 3 * 8) = (u64) payload;

	payload_size = gelic_recv_data(payload, PAYLOAD_SIZE - 3 * 8);
	if (payload_size <= 0)
		goto error;

	result = gelic_deinit();
	if (result != 0)
		goto error;

	result = mm_deinit();
	if (result != 0)
		goto error;

	__asm__ __volatile__ (
		"mtctr %0\n\t"
		"bctrl" : : "r"(payload));

	return 0;

done:

	lv1_panic(0);

	return 0;

error:

	lv1_panic(1);

	return -1;
}
