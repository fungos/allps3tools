
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

#ifndef _SC_ISO_H_
#define _SC_ISO_H_

#include <inttypes.h>

struct sc_iso_header
{
	u32 seqno;
	u32 mbmsg;
	u32 cmd;
	u32 cmd_size;
};

struct sc_iso_debug_buffer
{
	u8 *buf;
	u64 buf_size;
};

struct sc_iso_sc_binary_patch
{
	u64 data_size;
	u8 res[8];
	u8 data[0];
};

#endif
