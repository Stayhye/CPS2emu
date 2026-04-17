EE_BIN = bin/cps2emu.elf

# List all objects
EE_OBJS = src/getopt.o src/emumain.o src/emudraw.o src/font.o \
          src/zip/unzip.o src/zip/zfile.o src/sound/qsound.o src/sound/sndintrf.o \
          src/cps2/state.o src/cps2/cache.o src/cps2/cps2.o src/cps2/cps2crpt.o \
          src/cps2/driver.o src/cps2/eeprom.o src/cps2/inptport.o src/cps2/memintrf.o \
          src/cps2/sprite.o src/cps2/timer.o src/cps2/vidhrdw.o src/cps2/loadrom.o \
          src/cps2/coin.o src/cps2/hiscore.o src/cpu/m68000/c68k.o src/cpu/m68000/m68000.o \
          src/cpu/z80/cz80.o src/cpu/z80/z80.o

# Toolchain paths
EE_INCS = -Isrc -Isrc/cps2 -Isrc/zip -Isrc/sound -I$(PS2SDK)/ports/include -I$(PS2SDK)/ee/include -I$(PS2DEV)/gsKit/include
EE_LDFLAGS = -L$(PS2SDK)/ports/lib -L$(PS2SDK)/ee/lib -L$(PS2DEV)/gsKit/lib

# --- FIX: Library Order ---
# SDL and Audsrv must come BEFORE kernel/patches to ensure I/O redirection works.
# Added -lfileXio for better CD-ROM/ISO handling.
EE_LIBS = -lSDL -laudsrv -lpadx -lfileXio -lmc -lz -lm -lrt -lpatches -lkernel

# --- FIX: Define MAX_PATH and Force PS2 Context ---
# -DMAX_PATH=256 prevents string overflows on cdrom0:\\ paths.
# -O2 is used for performance, -G0 for small data optimization.
EE_CFLAGS = -DSDL -D_EE -DMAX_PATH=256 -std=gnu99 -O2 -G0 -Wall $(EE_INCS)

all: $(EE_BIN)

# Link rule
$(EE_BIN): $(EE_OBJS)
	mkdir -p bin
	$(EE_CC) $(EE_CFLAGS) $(EE_LDFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LIBS)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

# Include the official SDK build logic
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
