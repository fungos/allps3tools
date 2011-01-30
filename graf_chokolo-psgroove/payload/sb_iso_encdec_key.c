
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
#include <time.h>
#include <eid.h>
#include <spu.h>
#include <beep.h>
#include <sb_iso.h>
#include <sb_iso_encdec_key.h>

#define LDR_BASE					0x8000000014000000ULL
#define LDR_OFFSET					0x14000000ULL
#define LDR_PAGE_SIZE				16
#define LDR_SIZE					0x2300000
#define LDR_SIZE					0x2300000
#define METLDR_SIZE					0x100000
#define ISOLDR_SIZE					0x100000
#define RVKPRG_SIZE					0x100000
#define SPU_MODULE_SIZE				0x1000000

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

#define EID0_LS_ADDR				0x3E400
#define LDR_ARGS_LS_ADDR			0x3E800
#define RVK_PRG_LS_ADDR				0x3F000

struct ldr_args
{
	u64 prog_auth_id;
	u64 lpar_auth_id;
	void *spu_module;
	void *spu_module_arg1;
	u64 spu_module_arg1_size;
	void *spu_module_arg2;
	u64 spu_module_arg2_size;
	u8 res1[16];
	u64 field48;
	u8 res2[16];
};

static volatile u64 ldr_lpar_addr = 0x700020000000ULL + 0xE900000ULL - LDR_SIZE;

static volatile u64 lpar_auth_id = 0x1070000002000001;
static volatile u64 prog_auth_id = 0x1050000003000001;

static volatile u64 esid = 0x8000000018000000;
static volatile u64 vsid = 0x0000000000001400;

static struct ldr_args __attribute__ ((aligned ((128)))) ldr_args;

static volatile u64 __attribute__ ((aligned ((128)))) spu_ls_0x3EC00_value[12] = { -1 };

static volatile u64 __attribute__ ((aligned ((128)))) spu_ls_0x3E000_value[] =
	{ 0xFF00000000ULL, 0 };

static volatile u8 __attribute__ ((aligned ((128)))) debug_buffer[4096];

static volatile u8 __attribute__ ((aligned ((128)))) spu_args[0x100];

int sb_iso_encdec_key(void)
{
	u64 priv2_addr, problem_phys, local_store_phys, unused, shadow_addr, spe_id, intr_status;
	u8 *metldr, *isoldr, *rvkprg, *spu_module;
	struct spu_shadow volatile *spu_shadow;
	struct spu_problem volatile *spu_problem;
	struct spu_priv2 volatile *spu_priv2;
	u8 volatile *spu_ls;
	u64 ticks, key_size;
	u32 spu_out_intr_mbox_value;
	u32 spu_out_mbox_value;
	u8 mfc_cmd_tag;
	struct sb_iso_debug_buffer *sb_iso_debug_buffer;
	struct sb_iso_header *sb_iso_header;
	struct sb_iso_encdec_key *sb_iso_encdec_key;
 	int metldr_size, isoldr_size, rvkprg_size, spu_module_size, i, result;

	MM_LOAD_BASE(metldr, LDR_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) metldr), ldr_lpar_addr,
		LDR_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	metldr_size = gelic_recv_data(metldr, METLDR_SIZE);
	if (metldr_size <= 0)
		return metldr_size;

	isoldr = metldr + METLDR_SIZE;

	isoldr_size = gelic_recv_data(isoldr, ISOLDR_SIZE);
	if (isoldr_size <= 0)
		return isoldr_size;

	rvkprg = isoldr + ISOLDR_SIZE;

	rvkprg_size = gelic_recv_data(rvkprg, RVKPRG_SIZE);
	if (rvkprg_size <= 0)
		return rvkprg_size;

	spu_module = rvkprg + RVKPRG_SIZE;

	spu_module_size = gelic_recv_data(spu_module, SPU_MODULE_SIZE);
	if (spu_module_size <= 0)
		return spu_module_size;

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

	spu_in_mbox_write_64(spu_problem, (u64) isoldr);

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
					/* transfer EID0, ldr args and revoke list for programs into SPU LS */

					memset(spu_args, 0, sizeof(spu_args));
					sb_iso_debug_buffer = (struct sb_iso_debug_buffer *) spu_args;
					sb_iso_debug_buffer->buf = debug_buffer;
					sb_iso_debug_buffer->buf_size = sizeof(debug_buffer);

					memset(&ldr_args, 0, sizeof(ldr_args));
					ldr_args.prog_auth_id = prog_auth_id;
					ldr_args.lpar_auth_id = lpar_auth_id;
					ldr_args.spu_module = spu_module;
					ldr_args.spu_module_arg1 = spu_args;
					ldr_args.spu_module_arg1_size = sizeof(spu_args);
					ldr_args.spu_module_arg2 = eid4;
					ldr_args.spu_module_arg2_size = sizeof(eid4);
					ldr_args.field48 = 3;

					mfc_cmd_tag = 1;

					if (spu_mfc_cmd_exec(spu_problem, EID0_LS_ADDR,
						(u64) eid0, 0x400, mfc_cmd_tag, 0, MFC_CMD_GET) != 0)
						break;

					if (spu_mfc_cmd_exec(spu_problem, 0x3EC00,
						(u64) spu_ls_0x3EC00_value, sizeof(spu_ls_0x3EC00_value),
						mfc_cmd_tag, 0, MFC_CMD_GET) != 0)
						break;

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
					spu_out_mbox_value = spu_problem->spu_out_mbox;

					result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &spu_out_mbox_value, 4);
					if (result != 0)
						return result;

					if (spu_out_mbox_value != 2)
						break;
				}
				else if (spu_out_intr_mbox_value == 0x80)
				{
					key_size = 0xC0 / 8; /* 192 bits */

					sb_iso_header = (struct sb_iso_header *) spu_args;
					memset(sb_iso_header, 0, sizeof(struct sb_iso_header));
					sb_iso_header->seqno = 1;
					sb_iso_header->mbmsg = 1;
					sb_iso_header->cmd = 0x2004UL;
					sb_iso_header->cmd_size = sizeof(struct sb_iso_encdec_key) + key_size;

					sb_iso_encdec_key = (struct sb_iso_encdec_key *) (sb_iso_header + 1);
					memset(sb_iso_encdec_key, 0, sizeof(struct sb_iso_encdec_key) + key_size);
					sb_iso_encdec_key->field0 = 0x100 + 0;
					/*
					sb_iso_encdec_key->field0 = 0x110 + 0;
					*/
					sb_iso_encdec_key->field8 = SB_ISO_REPO_NODE_BUS_1_ID_V2;
					sb_iso_encdec_key->key_size = key_size;

					spu_in_mbox_write(spu_problem, 1);
				}
				else if (spu_out_intr_mbox_value == 0x81)
				{
					break;
				}
			}
		}

		if ((spu_problem->spu_status & 0x1) == 0)
			break;

		ticks = 1 * TB_TICKS_PER_SEC;

		sleep(ticks);
	}

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, debug_buffer, sizeof(debug_buffer));
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &spu_problem->spu_status, 4);
	if (result != 0)
		return result;

	result = gelic_xmit_data(gelic_bcast_mac_addr, 0xBEEF, spu_args, sizeof(spu_args));
	if (result != 0)
		return result;

	beep(BEEP_DOUBLE);

	lv1_panic(1);

    return 0;
}
