
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
#include <param.h>
#include <memset.h>
#include <sysmgr.h>
#include <time.h>
#include <beep.h>

static volatile struct sysmgr_header header;
static volatile struct sysmgr_ring_buzzer ring_buzzer;

int beep(int type)
{
	u64 nwritten;
	int result;

	if ((type != BEEP_SINGLE) &&
		(type != BEEP_DOUBLE) &&
		(type != BEEP_CONTINUOUS))
		return -1;

	memset(&header, 0, sizeof(struct sysmgr_header));
	header.version = 1;
	header.size = 16;
	header.payload_size = 8;
	header.sid = SYSMGR_SID_RING_BUZZER;

	memset(&ring_buzzer, 0, sizeof(struct sysmgr_ring_buzzer));

	switch (type)
	{
		case BEEP_SINGLE:
			ring_buzzer.field1 = 0x29;
			ring_buzzer.field2 = 0x4;
			ring_buzzer.field4 = 0x6;
		break;

		case BEEP_DOUBLE:
			ring_buzzer.field1 = 0x29;
			ring_buzzer.field2 = 0xA;
			ring_buzzer.field4 = 0x1B6;
		break;

		case BEEP_CONTINUOUS:
			ring_buzzer.field1 = 0x29;
			ring_buzzer.field2 = 0xA;
			ring_buzzer.field4 = 0xFFF;
		break;
	}

	result = lv1_write_virtual_uart(SYSMGR_VUART_PORT,
		param_lpar_addr + ((u64) &header - param_ea_addr),
		sizeof(struct sysmgr_header), &nwritten);
	if (result < 0)
		return result;

	result = lv1_write_virtual_uart(SYSMGR_VUART_PORT,
		param_lpar_addr + ((u64) &ring_buzzer - param_ea_addr),
		sizeof(struct sysmgr_ring_buzzer), &nwritten);
	if (result < 0)
		return result;

	sleep(1 * TB_TICKS_PER_SEC);

	return 0;
}
