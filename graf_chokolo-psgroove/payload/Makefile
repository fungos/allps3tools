
CC=ppu-gcc
STRIP=ppu-strip
OBJCOPY=ppu-objcopy
CFLAGS=-mno-toc -ffunction-sections -fdata-sections -Os -nostdinc $(INC)
LDFLAGS=-Tldscript.ld -nostartfiles -nostdlib -nodefaultlibs -Wl,--gc-sections
INC=-I.

SRC_ASM=\
	start.S \
	get_new_toc.S \
	hvcall.S \
	memcpy.S \
	memset.S \
	self_decrypter_hook_stage1.S \
	vuart_write_hook_stage1.S \
	vuart_read_hook_stage1.S \
	stor_read_hook_stage1.S \
	stor_write_hook_stage1.S \
	stor_send_command_hook_stage1.S \
	hvcall209_hook_stage1.S

SRC_C=\
	mm.c \
	param.c \
	gelic.c \
	patch_dispmgr.c \
	stor.c \
	encdec.c \
	vas.c \
	cksum.c \
	time.c \
	repo_node.c \
	dump_lv2.c \
	dump_slb.c \
	dump_sprg0.c \
	dump_htab.c \
	dump_stor.c \
	dump_flash.c \
	dump_repo_nodes.c \
	dump_profile.c \
	decrypt_profile.c \
	vuart.c \
	vuart_sysmgr.c \
	vuart_dispmgr.c \
	decrypt_usb_dongle_master_key.c \
	update_mgr_qa_flag.c \
	update_mgr_get_token_seed.c \
	update_mgr_set_token.c \
	update_mgr_inspect_pkg.c \
	query_lpar_address.c \
	eid.c \
	update_mgr_calc_token.c \
	update_mgr_verify_token.c \
	rvk_pkg.c \
	rvk_prg.c \
	decrypt_pkg.c \
	decrypt_self.c \
	decrypt_npdrm.c \
	dump_hvcall99_param.c \
	beep.c \
	self_decrypter_hook.c \
	decrypt_game.c \
	spu.c \
	decrypt_lv2_direct.c \
	sha1.c \
	usb_dongle_auth.c \
	product_mode_off.c \
	decrypt_self_direct.c \
	vuart_hook.c \
	stor_hook.c \
	hvcall209_hook.c \
	dump_lpar_ra.c \
	hv_mmap_exploit.c \
	dump_lv1.c \
	sc_mgr_read_eprom.c \
	sc_mgr_get_region_data.c \
	sc_mgr_get_sc_status.c \
	sc_mgr_get_srh.c \
	aim_get_device_type.c \
	aim_get_device_id.c \
	aim_get_ps_code.c \
	aim_get_open_ps_id.c \
	decrypt_profile_direct.c \
	sc_iso_sc_binary_patch.c \
	sc_iso_get_sc_status.c \
	sc_iso_get_property.c \
	sb_iso_get_rnd.c \
	sb_iso_encdec_key.c \
	edec_kgen1.c \
	store_file_on_flash.c \
	replace_lv2.c \
	dump_dev_flash.c \
	main.c

OBJ=$(SRC_ASM:.S=.o) $(SRC_C:.c=.o)

ELF=payload.elf
BIN=payload.bin

all: $(BIN)

$(BIN): $(ELF)
	$(STRIP) -s $^
	$(OBJCOPY) -j .text -j .data -j .got -j .opd -j .bss -O binary $^ $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f $(BIN) $(ELF) $(OBJ)
