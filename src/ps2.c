#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <kernel.h>
#include <sifrpc.h>
#include "emumain.h"

// Global directory strings used by the CPS2 engine
char launchDir[MAX_PATH];
char game_dir[MAX_PATH];
char cache_dir[MAX_PATH];

/**
 * Initializes the environment for the PS2.
 * We avoid loading custom IRXs here to prevent conflicts with 
 * the modules SDL already loads internally.
 */
void ps2_init_environment(void) {
    // Basic RPC init is still required for system calls
    SifInitRpc(0);

    // 1. Force the Launch Directory to the CD-ROM
    // This ensures fopen("./file") resolves to cdrom0:\file
    strcpy(launchDir, "cdrom0:\\");
    
    // 2. Set the ROM and Cache locations
    // We use the hardware prefixes directly
    strcpy(game_dir, "cdrom0:\\ROMS");
    strcpy(cache_dir, "mc1:");

    // 3. Set the Current Working Directory
    // Some versions of SDL and libc depend on this
    chdir("cdrom0:\\");

    // Standard Debug output to PCSX2 Console
    printf("[PS2] Environment set to cdrom0:\\\n");
    printf("[PS2] ROMS: %s | Cache: %s\n", game_dir, cache_dir);
}
