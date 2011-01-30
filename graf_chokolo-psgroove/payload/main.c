
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
#include <param.h>
#include <gelic.h>
#include <patch_dispmgr.h>
#include <dump_lv2.h>
#include <dump_slb.h>
#include <dump_sprg0.h>
#include <dump_htab.h>
#include <dump_stor.h>
#include <dump_flash.h>
#include <dump_repo_nodes.h>
#include <dump_profile.h>
#include <decrypt_profile.h>
#include <vuart_sysmgr.h>
#include <vuart_dispmgr.h>
#include <decrypt_usb_dongle_master_key.h>
#include <update_mgr_qa_flag.h>
#include <update_mgr_get_token_seed.h>
#include <update_mgr_set_token.h>
#include <update_mgr_inspect_pkg.h>
#include <query_lpar_address.h>
#include <update_mgr_calc_token.h>
#include <update_mgr_verify_token.h>
#include <decrypt_pkg.h>
#include <decrypt_self.h>
#include <decrypt_npdrm.h>
#include <self_decrypter_hook.h>
#include <decrypt_game.h>
#include <decrypt_lv2_direct.h>
#include <usb_dongle_auth.h>
#include <product_mode_off.h>
#include <decrypt_self_direct.h>
#include <vuart_hook.h>
#include <stor_hook.h>
#include <hvcall209_hook.h>
#include <dump_lpar_ra.h>
#include <hv_mmap_exploit.h>
#include <dump_lv1.h>
#include <sc_mgr_read_eprom.h>
#include <sc_mgr_get_region_data.h>
#include <sc_mgr_get_sc_status.h>
#include <sc_mgr_get_srh.h>
#include <aim_get_device_type.h>
#include <aim_get_device_id.h>
#include <aim_get_ps_code.h>
#include <aim_get_open_ps_id.h>
#include <decrypt_profile_direct.h>
#include <sc_iso_sc_binary_patch.h>
#include <sc_iso_get_sc_status.h>
#include <sc_iso_get_property.h>
#include <sb_iso_get_rnd.h>
#include <sb_iso_encdec_key.h>
#include <edec_kgen1.h>
#include <store_file_on_flash.h>
#include <replace_lv2.h>
#include <dump_dev_flash.h>

int main(void)
{
	int i, result;

	result = mm_init();
	if (result != 0)
		goto error;

	result = param_init();
	if (result != 0)
		goto error;

	result = gelic_init();
	if (result != 0)
		goto error;

	patch_dispmgr();

	/*
	result = gelic_xmit_test();
	if (result != 0)
		goto error;
	*/

	/*
	result = gelic_recv_test();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_lv2(0x8000000000000000ULL, 8 * 1024 * 1024);
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_slb();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_sprg0();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_htab();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_stor();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_flash();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_repo_nodes();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_repo_nodes_spu();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_profile();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_profile();
	if (result != 0)
		goto error;
	*/

	/*
	result = vuart_sysmgr();
	if (result != 0)
		goto error;
	*/

	/*
	result = vuart_dispmgr();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_usb_dongle_master_key();
	if (result != 0)
		goto error;
	*/

	/*
	result = update_mgr_qa_flag();
	if (result != 0)
		goto error;
	*/

	/*
	result = update_mgr_get_token_seed();
	if (result != 0)
		goto error;
	*/

	/*
	result = update_mgr_set_token();
	if (result != 0)
		goto error;
	*/

	/*
	result = update_mgr_inspect_pkg();
	if (result != 0)
		goto error;
	*/

	/*
	result = query_lpar_address();
	if (result != 0)
		goto error;
	*/

	/*
	result = update_mgr_calc_token();
	if (result != 0)
		goto error;
	*/

	/*
	result = update_mgr_verify_token();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_pkg();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_self();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_hvcall99_param();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_npdrm();
	if (result != 0)
		goto error;
	*/

	/*
	result = self_decrypter_hook();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_game();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_lv2_direct();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_lv2_direct_355();
	if (result != 0)
		goto error;
	*/

	/*
	result = usb_dongle_auth();
	if (result != 0)
		goto error;
	*/

	/*
	result = product_mode_off();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_self_direct();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_self_direct_355();
	if (result != 0)
		goto error;
	*/

	/*
	result = vuart_hook();
	if (result != 0)
		goto error;
	*/

	/*
	result = stor_hook();
	if (result != 0)
		goto error;
	*/

	/*
	result = hvcall209_hook();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_lpar_ra();
	if (result != 0)
		goto error;
	*/

	/*
	result = hv_mmap_exploit();
	if (result != 0)
		goto error;
	*/

	/*
	result = dump_lv1();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_mgr_read_eprom();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_mgr_get_region_data();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_mgr_get_sc_status();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_mgr_get_srh();
	if (result != 0)
		goto error;
	*/

	/*
	result = aim_get_device_type();
	if (result != 0)
		goto error;
	*/

	/*
	result = aim_get_device_id();
	if (result != 0)
		goto error;
	*/

	/*
	result = aim_get_ps_code();
	if (result != 0)
		goto error;
	*/

	/*
	result = aim_get_open_ps_id();
	if (result != 0)
		goto error;
	*/

	/*
	result = decrypt_profile_direct();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_iso_sc_binary_patch();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_iso_get_sc_status();
	if (result != 0)
		goto error;
	*/

	/*
	result = sc_iso_get_property();
	if (result != 0)
		goto error;
	*/

	/*
	result = sb_iso_get_rnd();
	if (result != 0)
		goto error;
	*/

	/*
	result = sb_iso_encdec_key();
	if (result != 0)
		goto error;
	*/

	/*
	result = edec_kgen1();
	if (result != 0)
		goto error;
	*/

	/*
	result = store_file_on_flash();
	if (result != 0)
		goto error;
	*/

	/*
	result = replace_lv2();
	if (result != 0)
		goto error;
	*/

	result = dump_dev_flash();
	if (result != 0)
		goto error;

	return 0;

done:

	lv1_panic(0);

	return 0;

error:

	lv1_panic(1);
}
