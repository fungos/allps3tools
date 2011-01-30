
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
#include <if_ether.h>
#include <memcpy.h>
#include <system.h>
#include <time.h>
#include <spu.h>
#include <elf.h>
#include <decrypt_game.h>

#define SELF_BASE					0x8000000014000000ULL
#define SELF_OFFSET					0x14000000ULL
#define SELF_PAGE_SIZE				16
#define SELF_SIZE					0x2800000
#define SELF_FILE_SIZE				0x1400000
#define SELF_DST_SIZE				(SELF_SIZE - SELF_FILE_SIZE)

#define SPU_SHADOW_BASE				0x80000000133D0000ULL
#define SPU_SHADOW_OFFSET			0x133D0000ULL
#define SPU_SHADOW_SIZE				0x1000

struct appldr_args
{
	u32 field0;
	u32 data_size;
	u8 *data;
	u16 field10;
	u8 field12;
	u8 field13;
	u32 field14;
	u64 field18;
	u64 field20;
	u64 field28;
	u64 field30;
	u64 field38;
};

struct spu_args
{
	u64 paid;
	u8 *sce_hdr;
	struct appldr_args **appldr_args;
	u64 segment;
	u8 *dst;
	u8 *field28;
	u64 field30;
	u64 field38;
	u64 field40[4];
	u8 field60[16];
	u8 field70[128];
};

static volatile u64 self_lpar_addr = 0x700020000000ULL + 0xE900000ULL - SELF_SIZE;

static volatile u64 paid = 0x10700005FF000001;
static volatile u64 esid = 0x8000000018000000;
static volatile u64 vsid = 0x0000000000001400;

static volatile u8 spu_args_field28[128] __attribute__ ((aligned ((128))));

static struct spu_args __attribute__ ((aligned ((128)))) spu_args;

static struct appldr_args __attribute__ ((aligned ((128)))) *appldr_args_ptr;

static struct appldr_args __attribute__ ((aligned ((128)))) appldr_args;

int decrypt_game(void)
{
	u64 priv2_addr, problem_phys, local_store_phys, unused, shadow_addr, spe_id, intr_status;
	u8 *self, *encrypted_segments, buf[VLAN_ETH_DATA_LEN];
	struct spu_shadow *spu_shadow;
	struct elf64_hdr *elf64_hdr;
	struct elf64_phdr *elf64_phdr;
	u64 segment_offset, segment_size, ticks, spu_out_intr_mbox_value;
	u16 proto;
 	int self_size, i, result;

	MM_LOAD_BASE(self, SELF_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) self), self_lpar_addr,
		SELF_SIZE, 0xC, 0, 0);
	if (result != 0)
		return result;

	self_size = gelic_recv_data(self, SELF_FILE_SIZE);
	if (self_size <= 0)
		return self_size;

	result = lv1_construct_logical_spe(0xC, 0xC, 0xC, 0xC, 0xC, vas_get_id(), 1,
		&priv2_addr, &problem_phys, &local_store_phys, &unused, &shadow_addr, &spe_id);
	if (result != 0)
		return result;

	ticks = 60 * TB_TICKS_PER_SEC;

	result = lv1_undocumented_function_138(spe_id, ticks);
	if (result != 0)
		return result;

	MM_LOAD_BASE(spu_shadow, SPU_SHADOW_OFFSET);

	result = mm_map_lpar_memory_region(0, MM_EA2VA((u64) spu_shadow), shadow_addr,
		SPU_SHADOW_SIZE, 0xC, 0, 0x3);
	if (result != 0)
		return result;

	elf64_hdr = (struct elf64_hdr *) (self + *(u64 *) (self + 0x30));
	elf64_phdr = (struct elf64_phdr *) (self + *(u64 *) (self + 0x38));
	encrypted_segments = self + *(u64 *) (self + 0x48);

	for (i = 0; i < elf64_hdr->e_phnum; i++)
	{
		if (((elf64_phdr[i].p_type != PT_LOAD) && (elf64_phdr[i].p_type != 0x700000A4)) ||
			((elf64_phdr[i].p_filesz == 0)))
			continue;

		appldr_args_ptr = 0;

		memset(&appldr_args, 0, sizeof(struct appldr_args));

		memset(spu_args_field28, 0, sizeof(spu_args_field28));
		memcpy(&spu_args_field28[32], self + (*(u64 *) (self + 0x58)) + 0x10, 0x60);

		memset(&spu_args, 0, sizeof(struct spu_args));
		spu_args.paid = paid;
		spu_args.sce_hdr = self;
		spu_args.appldr_args = &appldr_args_ptr;
		spu_args.segment = i;
		spu_args.dst = self + SELF_FILE_SIZE;
		spu_args.field28 = spu_args_field28;

		if (i == 0)
			spu_args.field30 = 0x2010000ULL;
		else
			spu_args.field30 = 0x2020000ULL;

		spu_args.field40[0] = 1ULL << 62;
		spu_args.field40[3] = 2;

		segment_offset = *(u64 *) (encrypted_segments + i * 0x20);
		segment_size = *(u64 *) (encrypted_segments + i * 0x20 + 8);

		result = lv1_undocumented_function_99(spe_id, (u64) &spu_args);
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

			result = lv1_get_spe_interrupt_status(spe_id, 2, &intr_status);
			if (result != 0)
				return result;

			if (intr_status & 0x1)
			{
				/* received a message from appldr */

				result = lv1_undocumented_function_167(spe_id, 0x4000, &spu_out_intr_mbox_value);
				if (result != 0)
					return result;

				/*
				result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, &spu_out_intr_mbox_value, 8);
				if (result != 0)
					return result;
				*/

				result = lv1_clear_spe_interrupt_status(spe_id, 2, intr_status, 0);
				if (result != 0)
					return result;

				appldr_args.field0 = 1;
				appldr_args.data_size = segment_size;
				appldr_args.data = self + segment_offset;
				appldr_args.field10 = 1;
				appldr_args.field12 = 1;
				appldr_args.field13 = 1;
				appldr_args.field14 = 0x1000;

				lwsync();

				appldr_args_ptr = &appldr_args;

				lwsync();
			}

			if ((spu_shadow->transition_notifier == 0x1) ||
				(spu_shadow->transition_notifier == 0x2) ||
				(spu_shadow->transition_notifier == 0x4) ||
				(spu_shadow->transition_notifier == 0x8))
				break;
		}

		result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, spu_shadow, SPU_SHADOW_SIZE);
		if (result != 0)
			return result;

		memset(buf, 0, sizeof(buf));
		memcpy(buf + 4, &i, 4);
		memcpy(buf + 8, &elf64_phdr[i].p_vaddr, 8);
		memcpy(buf + 16, &elf64_phdr[i].p_filesz, 8);
		memcpy(buf + 24, &elf64_phdr[i].p_memsz, 8);
		memcpy(buf + 32, &segment_offset, 8);
		memcpy(buf + 40, &segment_size, 8);

		result = gelic_xmit_data(gelic_bcast_mac_addr, 0xCAFE, buf, 0x40);
		if (result != 0)
			return result;

		/* use different protocol for each segment, makes easy to extract a segment from pcap dump */
		proto = 0xBEEF + i;

		result = gelic_xmit_data(gelic_bcast_mac_addr, proto, spu_args.dst, elf64_phdr[i].p_filesz);
		if (result != 0)
			return result;
	}

	beep(BEEP_DOUBLE);

	lv1_panic(1);

    return 0;
}
