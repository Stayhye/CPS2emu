#include <tamtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

/* Local Headers */
#include "emumain.h"
#include "inptport.h"
#include "getopt.h"

// Set global directories
char launchDir[MAX_PATH];
char game_dir[MAX_PATH] = "cdrom0:\\ROMS";
char cache_dir[MAX_PATH] = "mc1:"; 

void ps2_init_modules() {
    SifInitRpc(0);
    
    // Core ROM Modules
    SifLoadModule("rom0:LIBSD", 0, NULL);
    
    // Load Memory Card Drivers (Required for mc1: access)
    SifLoadModule("rom0:MCMAN", 0, NULL);
    SifLoadModule("rom0:MCSERV", 0, NULL);
    
    // Load Audio Driver from ISO
    SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    
    // Note: USBD and USBKBD are loaded here. 
    // The GitHub Action applies the Opcode NOP patch to prevent the hang.
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);
    
    // Settling time for IOP
    for(volatile int i = 0; i < 100000; i++) { __asm__("nop"); }
}

int main(int argc, char *argv[]) {
    // Force Launch Directory to CD-ROM to fix "host:/" errors
    strcpy(launchDir, "cdrom0:\\");
    chdir("cdrom0:\\");

    // Initialize RPC and load IRX modules
    ps2_init_modules();

    // High priority for the emulator thread
    ChangeThreadPriority(GetThreadId(), 72);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL Init Failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);

    // --- LOGGING & STABILITY FIX ---
    // Print status to PCSX2 Console for debugging
    printf("[PS2] LaunchDir: %s\n", launchDir);
    printf("[PS2] GameDir: %s\n", game_dir);
    printf("[PS2] Waiting for IOP to settle...\n");

    // Give the hardware a moment to breathe before jumping into the engine
    SDL_Delay(500); 

    printf("[PS2] Starting CPS2 Engine...\n");
    // -------------------------------

    cps2_main();
    
    return 0;
}
