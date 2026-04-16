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

/* Global Configuration */
char game_dir[MAX_PATH] = "cdrom0:\\ROMS";
char cache_dir[MAX_PATH] = "mc0:/CPS2CACHE"; // Redirected to Memory Card

/* PS2 Module Loading */
void ps2_init_modules() {
    SifInitRpc(0);

    // Initialize the IOP and load modules from CDROM
    // Note: Filenames on ISO are typically UPPERCASE
    SifLoadModule("rom0:LIBSD", 0, NULL);
    
    if (SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL) < 0) {
        printf("Error: Could not load AUDSRV.IRX\n");
    }
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    // Give the IOP time to settle after loading modules
    int i;
    for(i = 0; i < 100000; i++) { __asm__("nop"); }
}

int main(int argc, char *argv[]) {
    // Elevate thread priority for emulation stability
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);

    // Initialize PS2 Hardware Services
    ps2_init_modules();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL_Init Failed: %s\n", SDL_GetError());
        return -1;
    }

    // Disable keyboard events to prevent polling-related overhead/crashes
    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_EventState(SDL_KEYUP, SDL_IGNORE);

    // Set 16-bit Video Mode (320x240 is CPS2 Native)
    SDL_Surface* screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL) {
        return -1;
    }
    SDL_ShowCursor(SDL_DISABLE);

    // Setup Joysticks
    if (SDL_NumJoysticks() > 0) {
        SDL_JoystickOpen(0);
    }

    // Allocate core emulation buffers
    // upper_memory is used for ROM caching
    extern u8 *upper_memory;
    upper_memory = (u8 *)malloc(1024 * 1024 * 4); // 4MB buffer example

    // Enter CPS2 Core
    cps2_main();

    return 0;
}
