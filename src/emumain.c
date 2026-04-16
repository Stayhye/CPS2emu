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

// Set global directories
char game_dir[MAX_PATH] = "cdrom0:\\ROMS";
char cache_dir[MAX_PATH] = "mc1:"; // Forces cache logic to check Memory Card Slot 2

void ps2_init_modules() {
    SifInitRpc(0);
    
    // Core ROM Modules
    SifLoadModule("rom0:LIBSD", 0, NULL);
    
    // Load Memory Card Drivers (Required for mc1: access)
    SifLoadModule("rom0:MCMAN", 0, NULL);
    SifLoadModule("rom0:MCSERV", 0, NULL);
    
    // Load Audio Driver from ISO
    SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    
    // Note: USBD and USBKBD are loaded here, 
    // but the GitHub Action will now automatically NOP the hang in the ELF.
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);
    
    // Settling time for IOP
    for(volatile int i = 0; i < 100000; i++) { __asm__("nop"); }
}

int main(int argc, char *argv[]) {
    // The manual memory patch at 0x001465F8 has been removed.
    // The workflow now applies this patch to the binary automatically using the opcode pattern.

    ChangeThreadPriority(GetThreadId(), 72);
    ps2_init_modules();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) return -1;

    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);

    cps2_main();
    return 0;
}
