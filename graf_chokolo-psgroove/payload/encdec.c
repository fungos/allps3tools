
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
#include <memcpy.h>
#include <system.h>
#include <time.h>
#include <encdec.h>

#define ENCDEC_BUS_ID				4
#define ENCDEC_DEV_ID				0

#define ENCDEC_DMA_BASE				0x80000000133A0000ULL
#define ENCDEC_DMA_OFFSET			0x133A0000ULL
#define ENCDEC_DMA_PAGE_SIZE		12
#define ENCDEC_DMA_SIZE				(1 << ENCDEC_DMA_PAGE_SIZE)

u64 encdec_buf_lpar_addr;

u8 *encdec_buf;

int encdec_init(void)
{
	u64 muid, dma_bus_addr;
	int result;

	result = lv1_allocate_memory(ENCDEC_DMA_SIZE, ENCDEC_DMA_PAGE_SIZE, 0, 0,
		&encdec_buf_lpar_addr, &muid);
	if (result != 0)
		return result;

	result = lv1_allocate_device_dma_region(ENCDEC_BUS_ID, ENCDEC_DEV_ID,
		ENCDEC_DMA_SIZE, ENCDEC_DMA_PAGE_SIZE, 0, &dma_bus_addr);
	if (result != 0)
		return result;

	result = lv1_map_device_dma_region(ENCDEC_BUS_ID, ENCDEC_DEV_ID,
		encdec_buf_lpar_addr, dma_bus_addr, ENCDEC_DMA_SIZE, 0xF800000000000000ULL);
	if (result != 0)
		return result;

	MM_LOAD_BASE(encdec_buf, ENCDEC_DMA_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) encdec_buf), encdec_buf_lpar_addr,
		ENCDEC_DMA_SIZE, ENCDEC_DMA_PAGE_SIZE, 0, 0);
	if (result != 0)
		return result;

	memset(encdec_buf, 0, ENCDEC_DMA_SIZE);

	return 0;
}

int encdec_send_device_command(u64 cmd, u8 *send_data, u64 send_data_size,
	u8 *recv_data, u64 recv_data_size)
{
	u64 tag, status, ticks;
	int result;

	memset(encdec_buf, 0, ENCDEC_DMA_SIZE);

	memcpy(encdec_buf, send_data, send_data_size);

	result = lv1_storage_send_device_command(ENCDEC_DEV_ID, cmd,
		encdec_buf_lpar_addr, send_data_size,
		encdec_buf_lpar_addr + send_data_size, recv_data_size, &tag);
	if (result != 0)
		return result;

	for (;;)
	{
		result = lv1_storage_check_async_status(ENCDEC_DEV_ID, tag, &status);
		if (result != 0)
			continue;

		if (status == 0)
			break;

		ticks = TB_TICKS_PER_SEC / 10;

		sleep(ticks);
	}

	memcpy(recv_data, encdec_buf + send_data_size, recv_data_size);

	return 0;
}
