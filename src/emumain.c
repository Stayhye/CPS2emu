#include <tamtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <SDL/SDL.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

/* Local Headers */
#include "emumain.h"
#include "inptport.h"
#include "getopt.h"

// At the very start of main()
*(volatile uint32_t*)(0x001465F8) = 0x00000000;
// Make sure to sync instruction cache if needed
SyncDCache((void*)0x001465F8, (void*)0x001465FC);

char game_dir[MAX_PATH] = "cdrom0:\\ROMS";

void ps2_init_modules() {
    SifInitRpc(0);
    SifLoadModule("rom0:LIBSD", 0, NULL);
    SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);
    
    // Settling time for IOP
    for(volatile int i = 0; i < 100000; i++) { __asm__("nop"); }
}

int main(int argc, char *argv[]) {
    ChangeThreadPriority(GetThreadId(), 72);
    ps2_init_modules();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) return -1;

    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);

    cps2_main();
    return 0;
}
