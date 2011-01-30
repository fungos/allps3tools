
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

#include <system.h>
#include <spu.h>

void spu_slb_invalidate_all(struct spu_priv2 volatile *spu_priv2)
{
	int i;

	spu_priv2->slb_invalidate_all = 0;

	eieio();

	for (i = 0; i < SPU_SLB_MAX_ENTRIES; i++)
	{
		spu_priv2->slb_index = i;
		spu_priv2->slb_vsid = 0;
		spu_priv2->slb_esid = 0;

		eieio();
	}
}

int spu_slb_set_entry(struct spu_priv2 volatile *spu_priv2, u64 index, u64 esid, u64 vsid)
{
	if (index >= SPU_SLB_MAX_ENTRIES)
		return -1;

	spu_priv2->slb_index = index;
	spu_priv2->slb_vsid = vsid;

	eieio();

	spu_priv2->slb_esid = esid;

	eieio();

	return 0;
}

void spu_in_mbox_write(struct spu_problem volatile *spu_problem, u32 val)
{
	spu_problem->spu_in_mbox = val;

	eieio();
}

void spu_in_mbox_write_64(struct spu_problem volatile *spu_problem, u64 val)
{
	spu_problem->spu_in_mbox = val >> 32;
	spu_problem->spu_in_mbox = val & 0xFFFFFFFFUL;

	eieio();
}

void spu_sig_notify_1_2_write_64(struct spu_problem volatile *spu_problem, u64 val)
{
	spu_problem->spu_sig_notify_1 = val >> 32;
	spu_problem->spu_sig_notify_2 = val & 0xFFFFFFFFUL;

	eieio();
}

void spu_iso_load_req_enable(struct spu_priv2 volatile *spu_priv2)
{
	spu_priv2->spu_privcntl = 0x4;

	eieio();
}

void spu_iso_load_req(struct spu_problem volatile *spu_problem)
{
	spu_problem->spu_runcntl = SPU_RUNCNTL_ISO_LOAD_REQ;

	eieio();
}

void spu_stop_req(struct spu_problem volatile *spu_problem)
{
	spu_problem->spu_runcntl = SPU_RUNCNTL_STOP_REQ;

	eieio();
}

u8 spu_mbox_stat_intr_out_mbox_count(struct spu_problem volatile *spu_problem)
{
	return (((spu_problem->spu_mbox_stat) >> 16) & 0xFF);
}

u8 spu_mbox_stat_in_mbox_count(struct spu_problem volatile *spu_problem)
{
	return (((spu_problem->spu_mbox_stat) >> 8) & 0xFF);
}

u8 spu_mbox_stat_out_mbox_count(struct spu_problem volatile *spu_problem)
{
	return (spu_problem->spu_mbox_stat & 0xFF);
}

u8 spu_mfc_cmd_exec(struct spu_problem volatile *spu_problem, u32 lsa, u64 ea,
	u16 size, u16 tag, u16 classid, u16 cmd)
{
	spu_problem->mfc_lsa = lsa;
	spu_problem->mfc_eah = ea >> 32;
	spu_problem->mfc_eal = ea & 0xFFFFFFFFUL;
	spu_problem->mfc_size_tag = (size << 16) | tag;
	spu_problem->mfc_classid_cmd_cmdstatus = (classid << 16) | cmd;

	eieio();

	return (spu_problem->mfc_classid_cmd_cmdstatus & 0x3);
}

u8 spu_mfc_cmd_tag_status(struct spu_problem volatile *spu_problem, u8 tag)
{
	spu_problem->prxy_querytype = 0;
	spu_problem->prxy_querymask = 1 << tag;

	eieio();

	return (spu_problem->prxy_tagstatus >> tag) & 0x1;
}
