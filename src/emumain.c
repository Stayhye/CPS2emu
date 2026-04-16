/*****************************************************************************
    emumain.c - Unified PS2 Port
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

// Updated to point specifically to CDROM for PS2
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
    PS2 Module Loading (Fixes SifBindRpc and SDL Crashes)
******************************************************************************/
void ps2_init_modules() {
    SifInitRpc(0);

    // Load sound libraries from ROM and CDROM
    SifLoadModule("rom0:LIBSD", 0, NULL);
    if (SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL) < 0) {
        printf("Failed to load AUDSRV.IRX\n");
    }

    // Load USB drivers to satisfy SDL internal keyboard checks
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    // Give the IOP a moment to initialize the RPC services
    int i;
    for(i = 0; i < 1000000; i++) { __asm__("nop"); }
}

/******************************************************************************
    Main Entry Point
******************************************************************************/
int main(int argc, char *argv[]) {
    // 1. Thread Priority
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);

    // 2. Initialize IOP Modules (Must be before SDL_Init)
    ps2_init_modules();

    // 3. Initialize SDL with specific PS2 constraints
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    // Force SDL to ignore keyboard to prevent polling missing hardware
    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_EventState(SDL_KEYUP, SDL_IGNORE);

    // 4. Video Mode
    screen_surface = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen_surface == NULL) {
        return -1;
    }
    SDL_ShowCursor(SDL_DISABLE);

    // 5. Initialize Joystick (DualShock 2)
    if (SDL_NumJoysticks() > 0) {
        SDL_JoystickOpen(0);
    }

    //
