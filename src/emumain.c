/*****************************************************************************
    emumain.c - PS2 Port with CDROM and Sound Fixes
******************************************************************************/

#include "emumain.h"
#include "inptport.h"
#include "getopt.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <SDL/SDL.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

SDL_Surface* real_screen;

#define EMULATOR_TITLE      "CPS2EMU for PS2"
#define FRAMESKIP_LEVELS    12
#define TICKS_PER_SEC       1000000UL
#define TICKS_PER_FRAME     (TICKS_PER_SEC / FPS)

/******************************************************************************
    Global variables
******************************************************************************/
char game_name[16];
char parent_name[16];
char cache_parent_name[16];

// Updated for CD-ROM usage
char game_dir[MAX_PATH] = "cdrom0:\\ROMS";
char cache_dir[MAX_PATH] = "./cache";

int option_showfps;
int option_showtitle;
int option_speedlimit;
int option_autoframeskip;
int option_frameskip;
int option_rescale;
int option_screen_position;
int option_linescroll;
int option_fullcache;
int option_extinput;
int option_xorrom;
int option_tweak;
int option_cpuspeed;
int option_hiscore;

int option_sound_enable;
int option_samplerate;
int option_sound_volume;

int option_m68k_clock;
int option_z80_clock;

int machine_driver_type;
int machine_init_type;
int machine_input_type;
int machine_screen_type;
int machine_sound_type;

u32 frames_displayed;
int fatal_error;
volatile int Loop;
char launchDir[MAX_PATH] = {0, };

int state_slot = 0;
int service_mode = 0;
int menu_mode = 0;
u8 *upper_memory = NULL;
u16 *work_frame = NULL;

static SDL_Surface *screen_surface = NULL;
static u16 *screen = NULL;

/******************************************************************************
    PS2 Module Initialization (Fixes SifBindRpc/Sound)
******************************************************************************/
void ps2_init_modules() {
    SifInitRpc(0);

    // Initialize the IOP and load core sound/USB modules
    SifLoadModule("rom0:LIBSD", 0, NULL);
    
    // Load external drivers from the Disc - Must be UPPERCASE on ISO
    if (SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL) < 0) {
        printf("Failed to load AUDSRV.IRX\n");
    }
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    // Small delay to let IOP services settle
    nopdelay();
}

/******************************************************************************
    Main Entry Point
******************************************************************************/
int main(int argc, char *argv[]) {
    // 1. Thread Setup
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);

    // 2. IOP Setup (Crucial for SifBindRpc)
    ps2_init_modules();

    // 3. SDL Initialization
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL_Init Failed: %s\n", SDL_GetError());
        return -1;
    }

    // Ignore keyboard to prevent polling crashes
    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_EventState(SDL_KEYUP, SDL_IGNORE);

    // 4. Video Mode Setup
    screen_surface = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen_surface == NULL) {
        return -1;
    }
    SDL_ShowCursor(SDL_DISABLE);

    // 5. Joystick Setup
    if (SDL_NumJoysticks() > 0) {
        SDL_JoystickOpen(0);
    }

    // 6. Memory Allocation
    upper_memory = (u8 *)malloc(CACHE_SIZE);
    work_frame = (u16 *)memalign(64, BUF_WIDTH * BUF_HEIGHT * 2);

    // 7. Core Execution
    cps2_main();

    return 0;
}
