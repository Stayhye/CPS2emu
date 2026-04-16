EE_BIN = bin/cps2emu.elf
EE_OBJS = src/getopt.o src/emumain.o src/emudraw.o src/cps2/state.o src/font.o \
          src/zip/unzip.o src/zip/zfile.o src/sound/qsound.o src/sound/sndintrf.o \
          src/cps2/cache.o src/cps2/cps2.o src/cps2/cps2crpt.o src/cps2/driver.o \
          src/cps2/eeprom.o src/cps2/inptport.o src/cps2/memintrf.o src/cps2/sprite.o \
          src/cps2/timer.o src/cps2/vidhrdw.o src/cps2/loadrom.o src/cps2/coin.o \
          src/cps2/hiscore.o src/cpu/m68000/c68k.o src/cpu/m68000/m68000.o \
          src/cpu/z80/cz80.o src/cpu/z80/z80.o

# Toolchain paths
PS2SDK_LIBS = $(PS2SDK)/ee/lib
PS2SDK_INCS = $(PS2SDK)/ee/include $(PS2SDK)/common/include $(PS2SDK)/ports/include

EE_INCS = -I. -Isrc -Isrc/cps2 -I$(PS2SDK_INCS)
EE_LDFLAGS = -L$(PS2SDK_LIBS) -L$(PS2SDK)/ports/lib -mno-crt0 $(PS2SDK)/ee/startup/crt0.o
EE_LIBS = -lSDL -lz -lpatches -lgcc -lc -lkernel -ldebug -laudsrv

EE_CFLAGS = -D_EE -G0 -O2 -Wall -gdwarf-2 -gz -DSDL -std=gnu99 $(EE_INCS)

all: $(EE_BIN)

$(EE_BIN): $(EE_OBJS)
	$(EE_CC) $(EE_CFLAGS) $(EE_LDFLAGS) -o $(EE_BIN) $(EE_OBJS) $(EE_LIBS)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
