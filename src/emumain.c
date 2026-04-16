#include <tamtypes.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* PS2 SDK / SDL Headers */
#include <SDL/SDL.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

/* Local Project Headers */
#include "emumain.h"
#include "inptport.h"
#include "getopt.h"

char game_dir[MAX_PATH] = "cdrom0:\\ROMS";

/* Initialize IOP Modules */
void ps2_init_modules() {
    SifInitRpc(0);

    // Load basic sound and file services
    SifLoadModule("rom0:LIBSD", 0, NULL);
    
    // Load custom drivers from the CD/ISO
    if (SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL) < 0) {
        printf("Failed to load AUDSRV.IRX\n");
    }
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    // Wait for IOP to initialize
    int i;
    for(i = 0; i < 100000; i++) { __asm__("nop"); }
}

int main(int argc, char *argv[]) {
    // Increase priority for smooth emulation
    ChangeThreadPriority(GetThreadId(), 72);

    ps2_init_modules();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        return -1;
    }

    // PS2 Optimization: Ignore keys to save cycles
    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_EventState(SDL_KEYUP, SDL_IGNORE);

    // Setup screen: 320x240 is standard for CPS2
    SDL_Surface* screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == NULL) {
        return -1;
    }
    SDL_ShowCursor(SDL_DISABLE);

    if (SDL_NumJoysticks() > 0) {
        SDL_JoystickOpen(0);
    }

    // Call the core emulator loop
    cps2_main();

    return 0;
}
