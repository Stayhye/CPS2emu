# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: Makefile 1187 2005-06-27 17:34:43Z lukasz $
#CPS2 EMU

EE_CC = ee-gcc
EE_CXX = ee-g++
EE_AS = ee-as
EE_LD = ee-ld
EE_AR = ee-ar
EE_OBJCOPY = ee-objcopy
EE_STRIP = ee-strip
EE_BIN = bin/cps2emu.elf

# Redream (main engine) objet files codes
#src/emumain.o src/emudraw.o
OBJS =  \
src/getopt.o src/emumain.o src/emudraw.o src/cps2/state.o src/font.o src/zip/unzip.o src/zip/zfile.o \
src/sound/qsound.o src/sound/sndintrf.o src/cps2/cache.o src/cps2/cps2.o src/cps2/cps2crpt.o \
src/cps2/driver.o src/cps2/eeprom.o src/cps2/inptport.o src/cps2/memintrf.o src/cps2/sprite.o  src/cps2/timer.o \
src/cps2/vidhrdw.o src/cps2/loadrom.o src/cps2/coin.o src/cps2/hiscore.o \
src/cpu/m68000/c68k.o src/cpu/m68000/m68000.o \
src/cpu/z80/cz80.o src/cpu/z80/z80.o

DEFINES = -DSDL -DRS97
INCLUDES = -I. -Isrc -Isrc/cps2 
CFLAGS = $(DEFINES) $(INCLUDES) $(OPT_FLAGS) -std=gnu99 -DRS97
CXXFLAGS = $(DEFINES) $(INCLUDES) $(OPT_FLAGS) -std=gnu++98
LDFLAGS = -lSDL -lm -pthread -ldl -lstdc++ -lz

EE_OBJS = $(OBJS)
EE_INCS += $(INCLUDES)
EE_INCS += -I../../ -I$(PS2SDK)/ports/include
EE_LDFLAGS = -L../../lib/ -L$(PS2SDK)/ports/lib -L$(PS2DEV)/gsKit/lib
EE_LIBS = -L. -lc -L$(PS2DEV)/gsKit/lib -L../lib -lsdlmixer -lSDL_image -lpng -ljpeg -lsdl -lgskit -ldmakit -laudsrv -lc -lm
EE_INCS += -I./src/

EE_CXXFLAGS += $(CXXFLAGS)
EE_CFLAGS += $(CFLAGS)

all: $(EE_BIN)

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN) $(PS2DEV)/cps2/rom/megaman2

reset:
	ps2client reset

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

EE_OBJS = $(OBJS)
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
