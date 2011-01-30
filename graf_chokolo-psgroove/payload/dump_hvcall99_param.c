
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

#include <inttypes.h>
#include <mm.h>
#include <if_vlan.h>
#include <gelic.h>
#include <param.h>
#include <get_new_toc.h>
#include <dump_hvcall99_param.h>

#define INSTR_OFFSET1			0x0004FA0CULL
#define INSTR_OFFSET2			0x0007D8E8ULL
#define INSTR_OFFSET3			0x0000539CULL
#define INSTR_OFFSET4			0x00037E34ULL
#define INSTR_OFFSET5			0x000053A8ULL
#define INSTR_OFFSET6			0x000569ECULL
#define INSTR_OFFSET7			0x0004F0E0ULL
#define INSTR_OFFSET8			0x0004F940ULL
#define INSTR_OFFSET9			0x0004A27CULL

struct spu_args
{
	u64 paid;
	u8 *sce_hdr;
	u8 *field10;
	u64 segment;
	u8 *dst;
	u8 *field28;
	u64 field30;
	u64 field38;
	u64 field40[4];
	u8 field60[16];
	u8 field70[128];
};

static u32 patch[] =
{
	0x3BE00001,		/* li %r31, 1 */
	0x7BFFF806,		/* rldicr %r31, %r31, 63, 0 */
	0x67FF0000,		/* oris %r31, %r31, <high half word> */
	0x63FF0000,		/* ori %r31, %r31, <low half word> */
	0x7FE903A6, 	/* mtctr %r31 */
	0x4E800421,		/* bctrl */
};

static void send_hvcall99_param1(u64 arg1, u64 arg2, u64 arg3, u64 arg4);

static void send_hvcall99_param2(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);

static void send_hvcall99_param3(u64 arg1, u64 arg2, u64 arg3);

static void send_hvcall99_param4(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);

static void send_hvcall99_param5(void);

static void send_hvcall99_param6(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5,
	u64 arg6, u64 arg7);

static void send_hvcall99_param7(u64 arg1);

static void send_hvcall99_param8(void);

static void send_hvcall99_param9(u64 arg1);

int dump_hvcall99_param(void)
{
	u8 *instr_addr;
	u64 func_addr;

	MM_LOAD_BASE(instr_addr, INSTR_OFFSET9);

	func_addr = param_ea_addr + *(u64 *) send_hvcall99_param9;

	patch[2] |= ((u32) func_addr) >> 16;
	patch[3] |= ((u32) func_addr) & 0xFFFFUL;

	memcpy(instr_addr, patch, sizeof(patch));

	return 0;
}

static void send_hvcall99_param1(u64 arg1, u64 arg2, u64 arg3, u64 arg4)
{
	u64 new_toc;
	struct spu_args *spu_args;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	spu_args = (struct spu_args *) arg4;

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_args, sizeof(struct spu_args));
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_args->sce_hdr, 3 * VLAN_ETH_DATA_LEN);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_args->field10, 3 * VLAN_ETH_DATA_LEN);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_args->dst, 3 * VLAN_ETH_DATA_LEN);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_args->field28, 16 * 8);

	lv1_panic(1);
}

static void send_hvcall99_param2(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg5, 8);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, arg4, arg5 * 0x48);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *(u64 *) ((u8 *) arg4 + 0x20), VLAN_ETH_DATA_LEN);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *(u64 *) ((u8 *) arg4 + 0x28), VLAN_ETH_DATA_LEN);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *(u64 *) ((u8 *) arg4 + 0x38), VLAN_ETH_DATA_LEN);

	lv1_panic(1);
}

static void send_hvcall99_param3(u64 arg1, u64 arg2, u64 arg3)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, arg3, 0x30);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *(u64 *) arg3, VLAN_ETH_DATA_LEN);

	lv1_panic(1);
}

static void send_hvcall99_param4(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg1, 8);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg2, 8);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg3, 8);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg4, 8);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg5, 8);

	lv1_panic(1);
}

static void send_hvcall99_param5(void)
{
	u64 new_toc;
	u64 *ptr;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	__asm__ __volatile__ ("mr %0, %%r30" : "=r"(ptr));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *(ptr + 5), 20 * VLAN_ETH_DATA_LEN);

	lv1_panic(1);
}

static void send_hvcall99_param6(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5,
	u64 arg6, u64 arg7)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *((u64 *) arg7 + 3), VLAN_ETH_DATA_LEN);
	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, *((u64 *) *((u64 *) arg7 + 3) + 1),
		VLAN_ETH_DATA_LEN);

	lv1_panic(1);
}

static void send_hvcall99_param7(u64 arg1)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &arg1, 8);

	lv1_panic(1);
}

static void send_hvcall99_param8(void)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	lv1_panic(1);
}

static void send_hvcall99_param9(u64 arg1)
{
	u64 new_toc;

	new_toc = get_new_toc();

	__asm__ __volatile__ ("mr %%r2, %0" : : "r"(new_toc));

	gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, arg1, 0x40);

	lv1_panic(1);
}
