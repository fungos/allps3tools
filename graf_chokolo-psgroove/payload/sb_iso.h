
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

#ifndef _SB_ISO_H_
#define _SB_ISO_H_

#include <inttypes.h>

#define SB_ISO_REPO_NODE_BUS_1_ID_V2			0x3000103UL

struct sb_iso_header
{
	u32 seqno;
	u32 mbmsg;
	u32 cmd;
	u32 cmd_size;
};

struct sb_iso_debug_buffer
{
	u8 *buf;
	u64 buf_size;
};

struct sb_iso_get_rnd
{
	u32 field0;
	u8 res[12];
};

struct sb_iso_encdec_key
{
	u32 field0;
	u32 field8;
	u64 key_size;
	u8 field18[0];
};

#endif
