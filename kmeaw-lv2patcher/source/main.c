#include "common.h"
#include "peek_poke.h"
#include "hvcall.h"
#include "mm.h"

#include <psl1ght/lv2.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sysutil/video.h>
#include <rsx/gcm.h>
#include <rsx/reality.h>
#include <io/pad.h>
#include <sys/stat.h>

#include "sconsole.h"

u64 mmap_lpar_addr;

#define PATCH_FLAG_EXEC		1
#define PATCH_FLAG_BACKUP	2

static const char *search_dirs[13] = {
	"/dev_usb000/lv2/",
	"/dev_usb001/lv2/",
	"/dev_usb002/lv2/",
	"/dev_usb003/lv2/",
	"/dev_usb004/lv2/",
	"/dev_usb005/lv2/",
	"/dev_usb006/lv2/",
	"/dev_usb007/lv2/",

	"/dev_cf/lv2/",
	"/dev_sd/lv2/",
	"/dev_ms/lv2/",

	"/dev_hdd0/game/LV2000000/USRDIR/",

	NULL
};

unsigned char *read_file(FILE * f, size_t * sz)
{
	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	*sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	unsigned char *userlandBuffer = malloc(*sz);
	if (!userlandBuffer)
		return NULL;

	fread(userlandBuffer, 1, *sz, f);
	fclose(f);

	if (*((u32 *) userlandBuffer) == 0) {
		free(userlandBuffer);
		userlandBuffer = NULL;
	}

	return userlandBuffer;
}

char filename_buf[256];

FILE *search_file(char *filename)
{
	FILE *result;
	const char **search_dir = search_dirs;
	u32 test_value;

	if (*filename == '/')
		return fopen(filename, "r");

	while (*search_dir) {
		strcpy(filename_buf, *search_dir);
		strcat(filename_buf, filename);
		result = fopen(filename_buf, "r");
		if (result) {
			test_value = 0;
			fread(&test_value, sizeof(u32), 1, result);
			fseek(result, 0, SEEK_SET);
			if (test_value == 0) {
				fclose(result);
				result = NULL;
			}
		}
		if (result)
			return result;
		else
			search_dir++;
	}

	return NULL;
}

int map_lv1()
{
	int result =
	    lv1_undocumented_function_114(0, 0xC, HV_SIZE, &mmap_lpar_addr);
	if (result != 0) {
		PRINTF("Error code %d calling lv1_undocumented_function_114\n",
		       result);
		return 0;
	}

	result =
	    mm_map_lpar_memory_region(mmap_lpar_addr, HV_BASE, HV_SIZE, 0xC, 0);
	if (result) {
		PRINTF("Error code %d calling mm_map_lpar_memory_region\n",
		       result);
		return 0;
	}

	return 1;
}

void unmap_lv1()
{
	if (mmap_lpar_addr != 0)
		lv1_undocumented_function_115(mmap_lpar_addr);
}

void patch_lv2_protection()
{
	// changes protected area of lv2 to first byte only
	lv1_poke(0x363a78, 0x0000000000000001ULL);
	lv1_poke(0x363a80, 0xe0d251b556c59f05ULL);
	lv1_poke(0x363a88, 0xc232fcad552c80d7ULL);
	lv1_poke(0x363a90, 0x65140cd200000000ULL);
}

typedef struct {
	int height;
	int width;
	uint32_t *ptr;
	// Internal stuff
	uint32_t offset;
} buffer;

gcmContextData *context;
VideoResolution res;
int currentBuffer = 0;
buffer *buffers[2];

void waitFlip()
{
	// Block the PPU thread untill the previous flip operation has finished.
	while (gcmGetFlipStatus() != 0)
		usleep(200);
	gcmResetFlipStatus();
}

void flip(s32 buffer)
{
	assert(gcmSetFlip(context, buffer) == 0);
	realityFlushBuffer(context);
	gcmSetWaitFlip(context);
}

void makeBuffer(int id, int size)
{
	buffer *buf = malloc(sizeof(buffer));
	buf->ptr = rsxMemAlign(16, size);
	assert(buf->ptr != NULL);

	assert(realityAddressToOffset(buf->ptr, &buf->offset) == 0);
	assert(gcmSetDisplayBuffer
	       (id, buf->offset, res.width * 4, res.width, res.height) == 0);

	buf->width = res.width;
	buf->height = res.height;
	buffers[id] = buf;
}

void init_screen()
{
	void *host_addr = memalign(1024 * 1024, 1024 * 1024);
	assert(host_addr != NULL);

	context = realityInit(0x10000, 1024 * 1024, host_addr);
	assert(context != NULL);

	VideoState state;
	assert(videoGetState(0, 0, &state) == 0);
	assert(state.state == 0);

	assert(videoGetResolution(state.displayMode.resolution, &res) == 0);

	VideoConfiguration vconfig;
	memset(&vconfig, 0, sizeof(VideoConfiguration));
	vconfig.resolution = state.displayMode.resolution;
	vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
	vconfig.pitch = res.width * 4;

	assert(videoConfigure(0, &vconfig, NULL, 0) == 0);
	assert(videoGetState(0, 0, &state) == 0);

	s32 buffer_size = 4 * res.width * res.height;

	gcmSetFlipMode(GCM_FLIP_VSYNC);
	makeBuffer(0, buffer_size);
	makeBuffer(1, buffer_size);

	gcmResetFlipStatus();
	flip(1);
}

void go(void *addr)
{
	u64 syscall11_ptr = lv2_peek(SYSCALL_PTR(11));
	u64 old_syscall11 = lv2_peek(syscall11_ptr);
	lv2_poke(syscall11_ptr, (u64) addr);
	Lv2Syscall0(11);
	lv2_poke(syscall11_ptr, old_syscall11);
}

void install_lv2_memcpy()
{
	PRINTF("installing memcpy...\n");
	/* install memcpy */
	lv2_poke(NEW_POKE_SYSCALL_ADDR, 0x4800000428250000ULL);
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 8, 0x4182001438a5ffffULL);
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 16, 0x7cc428ae7cc329aeULL);
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 24, 0x4bffffec4e800020ULL);
}

void remove_lv2_memcpy()
{
	PRINTF("uninstalling memcpy...\n");
	/* restore syscall */
	remove_new_poke();
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 16, 0xebc2fe287c7f1b78);
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 24, 0x3860032dfba100e8);
}

inline static void lv2_memcpy(void *to, const void *from, size_t sz)
{
	Lv2Syscall3(NEW_POKE_SYSCALL, (unsigned long long)to,
		    (unsigned long long)
		    from, sz);
}

s32 main(s32 argc, const char *argv[])
{
	debug_wait_for_client();

	PRINTF("installing new poke syscall\n");
	install_new_poke();

	PRINTF("mapping lv1\n");
	if (!map_lv1()) {
		remove_new_poke();
		exit(0);
	} else {
		PRINTF("patching lv2 mem protection\n");
		patch_lv2_protection();

		PRINTF("removing new poke syscall\n");
		remove_new_poke();
	}

	PadInfo padinfo;
	PadData paddata;
	int i, j;
#define _poke(a,b)	lv2_poke (0x8000000000000000ULL + (u64) a, b)
#define _poke32(a,b)	lv2_poke32 (0x8000000000000000ULL + (u64) a, b)

	FILE *patch = search_file("patch.txt");
	FILE *payload;

	char buf[512], *ptr, *ptr2, flags = 0;
	char patch_path[256];
	unsigned long long value;
	void *addr, *backup_addr;
	int patches = 0, payloads = 0;

	if (patch) {
		strcpy(patch_path, filename_buf);
		while (!feof(patch)) {
			if (!fgets(buf, sizeof(buf), patch))
				break;
			if (!buf[0])
				break;

			/*
			 * address: [@[x][b]] { payload_name | poke32 | poke64 | "go" }
			 *
			 * 472461: xb payload.bin
			 * 28ca70: 37719461
			 * 7f918a: 16380059372ab00a
			 */

			ptr = strchr(buf, '#');
			if (ptr)
				*ptr = 0;
			ptr = buf;
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;
			if (!strchr("0123456789abcdefABCDEF", *ptr))
				continue;
			addr = (void *)strtoull(ptr, &ptr, 16);
			if (*ptr != ':')
				continue;
			else
				ptr++;
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;
			flags = 0;
			if (*ptr == '@') {
				ptr++;
				if (*ptr == 'x') {
					flags |= PATCH_FLAG_EXEC;
					ptr++;
				}
				if (*ptr == 'b') {
					flags |= PATCH_FLAG_BACKUP;
					ptr++;
				}
				while (*ptr == ' ' || *ptr == '\t')
					ptr++;
			}
			if (ptr[0] == 'g' && ptr[1] == 'o') {
				addr += 0x8000000000000000ULL;
				go (addr);
			} else if (!strchr("0123456789abcdefABCDEF", *ptr))
				do {
					ptr2 = strchr(ptr, '\n');
					if (ptr2)
						*ptr2 = 0;
					ptr2 = strchr(ptr, '\r');
					if (ptr2)
						*ptr2 = 0;
					ptr2 = strchr(ptr, ' ');
					if (ptr2)
						*ptr2 = 0;
					ptr2 = strchr(ptr, '\t');
					if (ptr2)
						*ptr2 = 0;
					payload = search_file(ptr);

					if (!payload) {
						PRINTF
						    ("Cannot open file \"%s\".\n",
						     ptr);
						break;
					}

					PRINTF("reading payload...\n");
					size_t sz;
					unsigned char *payload_bin =
					    read_file(payload, &sz);

					backup_addr = 0;
					if (!addr)
					{
						addr = lv2_alloc((sz + 7) & ~7, 0x27);
						if (!addr || (u32)(u64)addr == 0x8001003)
							break;
					}
					else {
						addr += 0x8000000000000000ULL;
						if (flags & PATCH_FLAG_BACKUP)
							backup_addr =
							    lv2_alloc(sz, 0x27);
					}

					install_lv2_memcpy();

					if (flags & PATCH_FLAG_BACKUP) {
						/* backup */
						PRINTF
						    ("backing up the data...\n");
						lv2_memcpy(backup_addr, addr,
							   sz);
					}

					/* copy the payload */
					PRINTF("copying the payload...\n");
					lv2_memcpy((void *)
						   addr, payload_bin, sz);
					remove_lv2_memcpy();

					if (flags & PATCH_FLAG_EXEC) {
						PRINTF
						    ("Executing the payload...\n");
						go(addr);
					}

					if (flags & PATCH_FLAG_BACKUP) {
						PRINTF
						    ("Restoring LV2 memory...\n");
						install_lv2_memcpy();
						lv2_memcpy(addr, backup_addr,
							   sz);
						remove_lv2_memcpy();
					}

					PRINTF("Done.\n");

					payloads++;
				} while (0);
			else {
				ptr2 = ptr;
				value = strtoull(ptr, &ptr, 16);

				patches++;

				if (ptr - ptr2 == 8) {
					_poke32(addr, value);
					PRINTF("poke32 %p %08llX\n",
					       (void *)addr, value);
				} else if (ptr - ptr2 == 16) {
					_poke(addr, value);
					PRINTF("poke64 %p %16llX\n",
					       (void *)addr, value);
				} else
					patches--;
			}
		}

		fclose(patch);
	}
#undef _poke
#undef _poke32

	init_screen();
	ioPadInit(7);
	/*
	   Init the console: arguments (background color, font color, framebuffer, screen width, screen height)
	   sconsoleInit(int bgColor, int fgColor, int screenWidth, int screenHeight)
	 */
	sconsoleInit(FONT_COLOR_BLACK, FONT_COLOR_WHITE, res.width, res.height);
	char ts[1000];

	while (1) {
		ioPadGetInfo(&padinfo);
		for (i = 0; i < MAX_PADS; i++) {
			if (padinfo.status[i]) {
				ioPadGetData(i, &paddata);
				if (paddata.BTN_CROSS)
					return 0;
			}
		}

		waitFlip();

		//background
		for (i = 0; i < res.height; i++) {
			for (j = 0; j < res.width; j++)
				buffers[currentBuffer]->ptr[i * res.width + j] =
				    FONT_COLOR_BLACK;
		}

		//Let's do some printing   
		print(50, 50, "Hello from Russia!", buffers[currentBuffer]->ptr);
		sprintf(ts,
			"Installed %d payloads, %d patches.",
			payloads, patches);
		print(50, 150, ts, buffers[currentBuffer]->ptr);
		if (patches)
			print(50, 250, patch_path, buffers[currentBuffer]->ptr);
		print(50, 450, "Press X to quit", buffers[currentBuffer]->ptr);

		flip(currentBuffer);
		currentBuffer = !currentBuffer;
	}

	PRINTF("done, exiting\n");
	return 0;
}
