EE_BIN = bin/cps2emu.elf

# List all objects
EE_OBJS = src/getopt.o src/emumain.o src/emudraw.o src/font.o \
          src/zip/unzip.o src/zip/zfile.o src/sound/qsound.o src/sound/sndintrf.o \
          src/cps2/state.o src/cps2/cache.o src/cps2/cps2.o src/cps2/cps2crpt.o \
          src/cps2/driver.o src/cps2/eeprom.o src/cps2/inptport.o src/cps2/memintrf.o \
          src/cps2/sprite.o src/cps2/timer.o src/cps2/vidhrdw.o src/cps2/loadrom.o \
          src/cps2/coin.o src/cps2/hiscore.o src/cpu/m68000/c68k.o src/cpu/m68000/m68000.o \
          src/cpu/z80/cz80.o src/cpu/z80/z80.o

# Toolchain paths - simplified to avoid the giant wall of text in your logs
EE_INCS = -Isrc -Isrc/cps2 -Isrc/zip -Isrc/sound -I$(PS2SDK)/ports/include
EE_LDFLAGS = -L$(PS2SDK)/ports/lib
EE_LIBS = -lSDL -lz -lpatches -laudsrv

# Standard PS2 flags
EE_CFLAGS = -DSDL -std=gnu99

all: $(EE_BIN)

# We use the internal SDK rule for the binary, but we ensure the bin dir exists
$(EE_BIN): $(EE_OBJS)
	mkdir -p bin
	$(EE_CC) $(EE_CFLAGS) $(EE_LDFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LIBS)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

# Include the official SDK build logic
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
