
CC=ppu-gcc
STRIP=ppu-strip
OBJCOPY=ppu-objcopy
CFLAGS=-mno-toc -ffunction-sections -fdata-sections -Os -nostdinc $(INC)
LDFLAGS=-Tldscript.ld -nostartfiles -nostdlib -nodefaultlibs -Wl,--gc-sections
INC=-I.

SRC_ASM=\
	start.S \
	hvcall.S \
	memcpy.S \
	memset.S

SRC_C=\
	mm.c \
	gelic.c \
	main.c

OBJ=$(SRC_ASM:.S=.o) $(SRC_C:.c=.o)

ELF=bootstrap.elf
BIN=bootstrap.bin

all: $(BIN)

$(BIN): $(ELF)
	$(STRIP) -s $^
	$(OBJCOPY) -j .text -j .data -j .got -j .bss -O binary $^ $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f $(BIN) $(ELF) $(OBJ)
