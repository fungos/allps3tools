
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
#include <gelic.h>
#include <vas.h>
#include <if_ether.h>
#include <memcpy.h>
#include <system.h>
#include <spu.h>
#include <eid.h>
#include <update_mgr_calc_token.h>

#define SPU_MODULE_BASE				0x8000000014000000ULL
#define SPU_MODULE_OFFSET			0x14000000ULL
#define SPU_MODULE_PAGE_SIZE		16
#define SPU_MODULE_SIZE				(4 << SPU_MODULE_PAGE_SIZE)

#define SPU_SHADOW_BASE				0x80000000133D0000ULL
#define SPU_SHADOW_OFFSET			0x133D0000ULL
#define SPU_SHADOW_SIZE				0x1000

#define SPU_PRIV2_BASE				0x80000000133E0000ULL
#define SPU_PRIV2_OFFSET			0x133E0000ULL
#define SPU_PRIV2_SIZE				0x20000

struct idps_cmd
{
	u8 res1[16];
	u32 cmd;
	u8 res2[108];
};

struct spu_args
{
	u64 field0;
	u8 *token;
	u64 token_size;
	u8 *seed;
	u64 seed_size;
	struct idps_cmd *cmd;
	u64 cmd_size;
	u8 *eid0;
	u64 eid0_size;
	u8 res[8];
};

static volatile u64 spu_lpar_addr = 0x700020000000ULL + 0xE900000ULL - SPU_MODULE_SIZE;

static volatile u64 paid = 0x1050000003000001;
static volatile u64 esid = 0x8000000018000000;
static volatile u64 vsid = 0x0000000000001400;

static struct idps_cmd __attribute__ ((aligned ((128)))) spu_idps_cmd;

static u8 spu_token[0x50] __attribute__ ((aligned ((128))));
/*
static u8 spu_token[0x50] __attribute__ ((aligned ((128)))) =
{
	0xF6, 0x58, 0xDB, 0xAC, 0x63, 0xEB, 0x47, 0x99, 0xE2, 0x63,
	0xC0, 0x10, 0x66, 0x42, 0x3D, 0xF7, 0x34, 0x29, 0x90, 0x61,
	0x23, 0xED, 0x89, 0xEC, 0x21, 0x9E, 0xE2, 0x8B, 0x83, 0xF9,
	0x87, 0x2F, 0x32, 0x50, 0xEC, 0xC3, 0xD0, 0x3D, 0xEA, 0x6E,
	0x14, 0xE0, 0x81, 0xA2, 0x67, 0xCE, 0x86, 0xF7, 0x7A, 0xFE,
	0xDF, 0x11, 0xAB, 0x39, 0xE1, 0xCE, 0x57, 0x06, 0x42, 0xC0,
	0x2B, 0xB2, 0x3F, 0x49, 0x04, 0xC7, 0xE7, 0x58, 0x70, 0x19,
	0x6A, 0xF1, 0xE4, 0x94, 0x32, 0x36, 0x61, 0xB0, 0xA6, 0xB5,
};
*/

static u8 spu_seed[0x50] __attribute__ ((aligned ((128))));
/*
static u8 spu_seed[0x50] __attribute__ ((aligned ((128)))) =
{
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x85,
	0x00, 0x0A, 0x14, 0x05, 0x67, 0xA0, 0x79, 0x37, 0xDC, 0x17,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
*/

static struct spu_args __attribute__ ((aligned ((128)))) spu_args;

int update_mgr_calc_token(void)
{
	u64 ppe_id;
	u64 priv2_addr, problem_phys, local_store_phys, unused, shadow_addr, spe_id, intr_status;
	u8 *spu_module;
	struct spu_shadow *spu_shadow;
	struct spu_priv2 *spu_priv2;
	volatile u64 dummy;
 	int spu_module_size, i, result;

	MM_LOAD_BASE(spu_module, SPU_MODULE_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_module), spu_lpar_addr,
		SPU_MODULE_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	spu_module_size = gelic_recv_data(spu_module, SPU_MODULE_SIZE);
	if (spu_module_size <= 0)
		return spu_module_size;

	result = lv1_construct_logical_spe(0xC, 0xC, 0xC, 0xC, 0xC, vas_get_id(), 2,
		&priv2_addr, &problem_phys, &local_store_phys, &unused, &shadow_addr, &spe_id);
	if (result != 0)
		return result;

	MM_LOAD_BASE(spu_shadow, SPU_SHADOW_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_shadow), shadow_addr,
		SPU_SHADOW_SIZE, 0xC, 0, 0x3);
	if (result != 0)
		return result;

	memset(&spu_idps_cmd, 0, sizeof(spu_idps_cmd));
	spu_idps_cmd.cmd = 0x2;

	memset(spu_token, 0, 0x50);
	memset(spu_seed, 0, 0x50);

	memset(&spu_args, 0, sizeof(spu_args));
	spu_args.field0 = 0x1;
	spu_args.token = spu_token;
	spu_args.token_size = 0x50;
	spu_args.seed = spu_seed;
	spu_args.seed_size = 0x50;
	spu_args.cmd = &spu_idps_cmd;
	spu_args.cmd_size = 0x80;
	spu_args.eid0 = eid0;
	spu_args.eid0_size = sizeof(eid0);

	result = lv1_undocumented_function_209(spe_id, paid, (u64) spu_module,
		(u64) &spu_args, sizeof(spu_args), 0, 0, 6);
	if (result != 0)
		return result;

	result = lv1_undocumented_function_62(spe_id, 0, 0, esid, vsid);
	if (result != 0)
		return result;

	result = lv1_clear_spe_interrupt_status(spe_id, 1, intr_status, 0);
	if (result != 0)
		return result;

	result = lv1_undocumented_function_168(spe_id, 0x3000, 1ULL << 32);
	if (result != 0)
		return result;

	while (1)
	{
		result = lv1_get_spe_interrupt_status(spe_id, 1, &intr_status);
		if (result != 0)
			return result;

		if (intr_status)
		{
			result = lv1_undocumented_function_62(spe_id, 0, 0, esid, vsid);
			if (result != 0)
				return result;

			result = lv1_clear_spe_interrupt_status(spe_id, 1, intr_status, 0);
			if (result != 0)
				return result;

			result = lv1_undocumented_function_168(spe_id, 0x3000, 1ULL << 32);
			if (result != 0)
				return result;
		}

		if (spu_shadow->execution_status == 0x7)
		{
			MM_LOAD_BASE(spu_priv2, SPU_PRIV2_OFFSET);

			result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_priv2), priv2_addr,
				SPU_PRIV2_SIZE, 0xC, 0, 0x3);
			if (result != 0)
				return result;

			result = lv1_get_spe_interrupt_status(spe_id, 2, &intr_status);
			if (result != 0)
				return result;

			dummy = spu_priv2->spu_out_intr_mbox;

			result = lv1_undocumented_function_167(spe_id, 0x4000, &dummy);
			if (result != 0)
				return result;

			result = lv1_clear_spe_interrupt_status(spe_id, 2, intr_status, 0);
			if (result != 0)
				return result;

			result = lv1_undocumented_function_200(spe_id);
			if (result != 0)
				return result;
		}

		if ((spu_shadow->execution_status == 0xB) ||
			(spu_shadow->execution_status == 0x3))
			break;
	}

	lv1_pause(0);

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_shadow, SPU_SHADOW_SIZE);
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_token, sizeof(spu_token));
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_seed, sizeof(spu_seed));
	if (result != 0)
		return result;

	lv1_panic(1);

    return 0;
}
