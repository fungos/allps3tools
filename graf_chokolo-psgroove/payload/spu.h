
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

#ifndef _SPU_H_
#define _SPU_H_

#include <inttypes.h>

#define MFC_SR1								0x0000

struct spu_shadow
{
	u8 padding_0x0000_0x0140[0x0140 - 0x0000];
	u64 int_status_class0;
	u64 int_status_class1;
	u64 int_status_class2;
	u8 padding_0x0158_0x0610[0x0610 - 0x0158];
	u64 mfc_dsisr;
	u8 padding_0x0618_0x0620[0x0620 - 0x0618];
	u64 mfc_dar;
	u8 padding_0x0628_0x0800[0x0800 - 0x0628];
	u64 mfc_dsipr;
	u8 padding_0x0808_0x0810[0x0810 - 0x0808];
	u64 mfc_lscrr;
	u8 padding_0x0818_0x0C00[0x0C00 - 0x0818];
	u64 mfc_cer;
	u8 padding_0x0C08_0x0F00[0x0F00 - 0x0C08];
	u64 execution_status;
	u8 padding_0x0F08_0x0F10[0x0F10 - 0x0F08];
	u64 transition_notifier;
	u8 padding_0x0F08_0x1000[0x1000 - 0x0F18];
};

#define SPU_SLB_MAX_ENTRIES					8

struct spu_priv2
{
	u8 padding_0x00000_0x01108[0x01108 - 0x00000];
	u64 slb_index;
	u64 slb_esid;
	u64 slb_vsid;
	u64 slb_invalidate_entry;
	u64 slb_invalidate_all;
	u8 padding_0x01130_0x04000[0x04000 - 0x01130];
	u64 spu_out_intr_mbox;
	u8 padding_0x04008_0x04040[0x04040 - 0x04008];
	u64 spu_privcntl;
	u8 padding_0x04048_0x04060[0x04060 - 0x04048];
	u64 spu_chnlindex;
	u64 spu_chnlcnt;
	u64 spu_chnldata;
	u64 spu_cfg;
	u8 padding_0x04080_0x20000[0x20000 - 0x04080];
};

#define MFC_CMD_PUT							0x20
#define MFC_CMD_GET							0x40

#define SPU_RUNCNTL_STOP_REQ				0x0
#define SPU_RUNCNTL_ISO_LOAD_REQ			0x3

struct spu_problem
{
	u8 padding_0x00000_0x03004[0x03004 - 0x00000];
	u32 mfc_lsa;
	u32 mfc_eah;
	u32 mfc_eal;
	u32 mfc_size_tag;
	u32 mfc_classid_cmd_cmdstatus;
	u8 padding_0x03018_0x03104[0x03104 - 0x03018];
	u32 mfc_qstatus;
	u8 padding_0x03108_0x03204[0x03204 - 0x03108];
	u32 prxy_querytype;
	u8 padding_0x03208_0x0321C[0x0321C - 0x03208];
	u32 prxy_querymask;
	u8 padding_0x03220_0x0322C[0x0322C - 0x03220];
	u32 prxy_tagstatus;
	u8 padding_0x03230_0x04004[0x04004 - 0x03230];
	u32 spu_out_mbox;
	u8 padding_0x04008_0x0400C[0x0400C - 0x04008];
	u32 spu_in_mbox;
	u8 padding_0x04010_0x04014[0x04014 - 0x04010];
	u32 spu_mbox_stat;
	u8 padding_0x04018_0x0401C[0x0401C - 0x04018];
	u32 spu_runcntl;
	u8 padding_0x04020_0x04024[0x04024 - 0x04020];
	u32 spu_status;
	u8 padding_0x04028_0x1400C[0x1400C - 0x04028];
	u32 spu_sig_notify_1;
	u8 padding_0x14010_0x1C00C[0x1C00C - 0x14010];
	u32 spu_sig_notify_2;
	u8 padding_0x1C010_0x20000[0x20000 - 0x1C010];
};

void spu_slb_invalidate_all(struct spu_priv2 volatile *spu_priv2);

int spu_slb_set_entry(struct spu_priv2 volatile *spu_priv2, u64 index, u64 esid, u64 vsid);

void spu_in_mbox_write(struct spu_problem volatile *spu_problem, u32 val);

void spu_in_mbox_write_64(struct spu_problem volatile *spu_problem, u64 val);

void spu_sig_notify_1_2_write_64(struct spu_problem volatile *spu_problem, u64 val);

void spu_iso_load_req_enable(struct spu_priv2 volatile *spu_priv2);

void spu_iso_load_req(struct spu_problem volatile *spu_problem);

void spu_stop_req(struct spu_problem volatile *spu_problem);

u8 spu_mbox_stat_intr_out_mbox_count(struct spu_problem volatile *spu_problem);

u8 spu_mbox_stat_in_mbox_count(struct spu_problem volatile *spu_problem);

u8 spu_mbox_stat_out_mbox_count(struct spu_problem volatile *spu_problem);

u8 spu_mfc_cmd_exec(struct spu_problem volatile *spu_problem, u32 lsa, u64 ea,
	u16 size, u16 tag, u16 classid, u16 cmd);

u8 spu_mfc_cmd_tag_status(struct spu_problem volatile *spu_problem, u8 tag);

#endif
