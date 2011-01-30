
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
#include <dump_htab.h>

int dump_htab(void)
{
	void *htab;
	int result;

	MM_LOAD_BASE(htab, GAMEOS_HTAB_OFFSET);

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, htab, GAMEOS_HTAB_SIZE);
	if (result < 0)
		return result;

	return 0;
}
