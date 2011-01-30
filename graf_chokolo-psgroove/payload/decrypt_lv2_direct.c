
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
#include <beep.h>
#include <vas.h>
#include <memcpy.h>
#include <system.h>
#include <time.h>
#include <spu.h>
#include <decrypt_lv2_direct.h>

#define LDR_BASE					0x8000000014000000ULL
#define LDR_OFFSET					0x14000000ULL
#define LDR_PAGE_SIZE				16
#define LDR_SIZE					0x1000000
#define METLDR_SIZE					0x100000
#define LV2LDR_SIZE					0x100000
#define RVKPRG_SIZE					0x100000
#define LV2_SIZE					0x200000
#define DST_SIZE					(LDR_SIZE - METLDR_SIZE - LV2LDR_SIZE - RVKPRG_SIZE - LV2_SIZE)

#define SPU_SHADOW_BASE				0x80000000133D0000ULL
#define SPU_SHADOW_OFFSET			0x133D0000ULL
#define SPU_SHADOW_SIZE				0x1000

#define SPU_PROBLEM_BASE			0x8000000013400000ULL
#define SPU_PROBLEM_OFFSET			0x13400000ULL
#define SPU_PROBLEM_SIZE			0x20000

#define SPU_PRIV2_BASE				0x8000000013420000ULL
#define SPU_PRIV2_OFFSET			0x13420000ULL
#define SPU_PRIV2_SIZE				0x20000

#define SPU_LS_BASE					0x8000000013440000ULL
#define SPU_LS_OFFSET				0x13440000ULL
#define SPU_LS_SIZE					0x40000

#define LDR_ARGS_LS_ADDR			0x3E800
#define RVK_PRG_LS_ADDR				0x3F000

struct ldr_args
{
	u64 lpar_auth_id;
	u8 *lv2;
	u8 *dst;
	u64 field18;
	u8 res1[40];
	u64 field48;
	u8 res2[16];
};

static volatile u64 ldr_lpar_addr = 0x700020000000ULL + 0xE900000ULL - LDR_SIZE;

static volatile u64 lv2_kernel_lpar_auth_id = 0x1070000002000001;
static volatile u64 ps2emu_lpar_auth_id = 0x1020000003000001;

static volatile u64 esid = 0x8000000018000000;
static volatile u64 vsid = 0x0000000000001400;

static struct ldr_args __attribute__ ((aligned ((128)))) ldr_args;

static volatile u64 __attribute__ ((aligned ((128)))) spu_ls_0x3E000_value[] =
	{ 0xFF00000000ULL, 0 };

int decrypt_lv2_direct(void)
{
	u64 priv2_addr, problem_phys, local_store_phys, unused, shadow_addr, spe_id, intr_status;
	u8 *metldr, *lv2ldr, *rvkprg, *lv2;
	struct spu_shadow volatile *spu_shadow;
	struct spu_problem volatile *spu_problem;
	struct spu_priv2 volatile *spu_priv2;
	u8 volatile *spu_ls;
	u32 spu_out_intr_mbox_value;
	u32 spu_out_mbox_value;
	u8 mfc_cmd_tag;
	u64 ticks;
 	int metldr_size, lv2ldr_size, rvkprg_size, lv2_size, result;

	MM_LOAD_BASE(metldr, LDR_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) metldr), ldr_lpar_addr,
		LDR_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	metldr_size = gelic_recv_data(metldr, METLDR_SIZE);
	if (metldr_size <= 0)
		return metldr_size;

	lv2ldr = metldr + METLDR_SIZE;

	lv2ldr_size = gelic_recv_data(lv2ldr, LV2LDR_SIZE);
	if (lv2ldr_size <= 0)
		return lv2ldr_size;

	rvkprg = lv2ldr + LV2LDR_SIZE;

	rvkprg_size = gelic_recv_data(rvkprg, RVKPRG_SIZE);
	if (rvkprg_size <= 0)
		return rvkprg_size;

	lv2 = rvkprg + RVKPRG_SIZE;

	lv2_size = gelic_recv_data(lv2, LV2_SIZE);
	if (lv2_size <= 0)
		return lv2_size;

	result = lv1_construct_logical_spe(0xC, 0xC, 0xC, 0xC, 0xC, vas_get_id(), 0,
		&priv2_addr, &problem_phys, &local_store_phys, &unused, &shadow_addr, &spe_id);
	if (result != 0)
		return result;

	result = lv1_enable_logical_spe(spe_id, 6);
	if (result != 0)
		return result;

	result = lv1_set_spe_interrupt_mask(spe_id, 0, 0x7);
	if (result != 0)
		return result;

	result = lv1_set_spe_interrupt_mask(spe_id, 1, 0xF);
	if (result != 0)
		return result;

	result = lv1_set_spe_interrupt_mask(spe_id, 2, 0xF);
	if (result != 0)
		return result;

	MM_LOAD_BASE(spu_shadow, SPU_SHADOW_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_shadow), shadow_addr,
		SPU_SHADOW_SIZE, 0xC, 0, 0x3);
	if (result != 0)
		return result;

	MM_LOAD_BASE(spu_problem, SPU_PROBLEM_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_problem), problem_phys,
		SPU_PROBLEM_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	MM_LOAD_BASE(spu_priv2, SPU_PRIV2_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_priv2), priv2_addr,
		SPU_PRIV2_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	MM_LOAD_BASE(spu_ls, SPU_LS_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_ls), local_store_phys,
		SPU_LS_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	result = lv1_set_spe_privilege_state_area_1_register(spe_id, MFC_SR1, 0x10);
	if (result != 0)
		return result;

	spu_slb_invalidate_all(spu_priv2);

	spu_slb_set_entry(spu_priv2, 0, esid, vsid);

	spu_priv2->spu_cfg = 0;

	eieio();

	spu_in_mbox_write_64(spu_problem, (u64) lv2ldr);

	spu_sig_notify_1_2_write_64(spu_problem, (u64) metldr);

	spu_iso_load_req_enable(spu_priv2);

	spu_iso_load_req(spu_problem);

	while (1)
	{
		result = lv1_get_spe_interrupt_status(spe_id, 0, &intr_status);
		if (result != 0)
			return result;

		if (intr_status)
		{
			result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &intr_status, 8);
			if (result != 0)
				return result;

			result = lv1_clear_spe_interrupt_status(spe_id, 0, intr_status, 0);
			if (result != 0)
				return result;
		}

		result = lv1_get_spe_interrupt_status(spe_id, 1, &intr_status);
		if (result != 0)
			return result;

		if (intr_status)
		{
			result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &intr_status, 8);
			if (result != 0)
				return result;

			result = lv1_clear_spe_interrupt_status(spe_id, 1, intr_status, 0);
			if (result != 0)
				return result;
		}

		result = lv1_get_spe_interrupt_status(spe_id, 2, &intr_status);
		if (result != 0)
			return result;

		if (intr_status & 0x1)
		{
			/* mailbox interrupt */

			result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &intr_status, 8);
			if (result != 0)
				return result;

			if (spu_mbox_stat_intr_out_mbox_count(spu_problem) != 0)
			{
				spu_out_intr_mbox_value = spu_priv2->spu_out_intr_mbox;

				result = lv1_clear_spe_interrupt_status(spe_id, 2, intr_status, 0);
				if (result != 0)
					return result;

				result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &spu_out_intr_mbox_value, 4);
				if (result != 0)
					return result;

				if (spu_out_intr_mbox_value == 1)
				{
					/* transfer ldr args and revoke list for programs into SPU LS */

					memset(&ldr_args, 0, sizeof(ldr_args));
					ldr_args.lpar_auth_id = lv2_kernel_lpar_auth_id;
					/*
					ldr_args.lpar_auth_id = ps2emu_lpar_auth_id;
					*/
					ldr_args.lv2 = lv2;
					ldr_args.dst = lv2 + LV2_SIZE;
					ldr_args.field18 = -1;
					ldr_args.field48 = 1;

					mfc_cmd_tag = 1;

					if (spu_mfc_cmd_exec(spu_problem, LDR_ARGS_LS_ADDR,
						(u64) &ldr_args, sizeof(ldr_args), mfc_cmd_tag, 0, MFC_CMD_GET) != 0)
						break;

					if (spu_mfc_cmd_exec(spu_problem, RVK_PRG_LS_ADDR,
						(u64) rvkprg, *(u64 *) (rvkprg + 0x10) + *(u64 *) (rvkprg + 0x18),
						mfc_cmd_tag, 0, MFC_CMD_GET) != 0)
						break;

					if (spu_mfc_cmd_exec(spu_problem, 0x3E000,
						(u64) spu_ls_0x3E000_value, sizeof(spu_ls_0x3E000_value),
						mfc_cmd_tag, 0, MFC_CMD_GET) != 0)
						break;

					/* wait until MFC transfers are finished */

					while (spu_mfc_cmd_tag_status(spu_problem, mfc_cmd_tag) == 0)
						;

					eieio();

					if (spu_mbox_stat_out_mbox_count(spu_problem) == 0)
						break;

					spu_out_mbox_value = spu_problem->spu_out_mbox;

					result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &spu_out_mbox_value, 4);
					if (result != 0)
						return result;

					if (spu_out_mbox_value != 1)
						break;
				}
				else if (spu_out_intr_mbox_value == 2)
				{
					spu_stop_req(spu_problem);

					spu_out_mbox_value = spu_problem->spu_out_mbox;

					result = lv1_clear_spe_interrupt_status(spe_id, 2, intr_status, 0);
					if (result != 0)
						return result;

					result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &spu_out_mbox_value, 4);
					if (result != 0)
						return result;

					if (spu_out_mbox_value != 2)
						break;
				}
				else
				{
					result = lv1_clear_spe_interrupt_status(spe_id, 2, intr_status, 0);
					if (result != 0)
						return result;
				}
			}
		}

		if ((spu_problem->spu_status & 0x1) == 0)
			break;

		ticks = 1 * TB_TICKS_PER_SEC;

		sleep(ticks);
	}

	/*
	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 0, spu_problem, SPU_PROBLEM_SIZE);
	if (result != 0)
		return result;
	*/

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 0, &spu_problem->spu_status, 4);
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF + 1, ldr_args.dst, DST_SIZE);
	if (result != 0)
		return result;

	beep(BEEP_DOUBLE);

	lv1_panic(1);

    return 0;
}
