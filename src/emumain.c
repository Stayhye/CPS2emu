/******************************************************************************
    emumain.c
    Main Emulator Logic & Path Overrides
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emumain.h"
#include "common.h"

/* Global Paths - Forced to Hardware */
char game_dir[MAX_PATH] = "cdrom0:\\ROMS";
char cache_dir[MAX_PATH] = "mc1:";
char save_dir[MAX_PATH] = "mc1:/CPS2_SAVE";

/* This is the global ROM name. Force it to empty so menu triggers. */
char game_name[16] = "";

/* FORCE_CDROM_PATH
   This replaces the standard fopen to prevent hardcoded "host:" calls 
   from escaping to the IOP.
*/
FILE* ps2_fopen_override(const char* filename, const char* mode) {
    char final_path[MAX_PATH];

    // If the path already has a device prefix, use it.
    if (strstr(filename, ":")) {
        // If it's trying to use host, redirect it to cdrom
        if (strncmp(filename, "host", 4) == 0) {
            snprintf(final_path, MAX_PATH, "cdrom0:\\%s", strchr(filename, ':') + 1);
        } else {
            strncpy(final_path, filename, MAX_PATH);
        }
    } else {
        // No prefix? Force it to the ROMS directory on CD
        snprintf(final_path, MAX_PATH, "cdrom0:\\ROMS\\%s", filename);
    }

    printf("[I/O] Redirected Open: %s\n", final_path);
    return fopen(final_path, mode);
}

// Map the override
#define fopen ps2_fopen_override

void emu_init_globals() {
    // Zero out everything to prevent "last played" auto-loads
    memset(game_name, 0, sizeof(game_name));
    emu_status = EMU_MENU;
    
    // Ensure directories are correctly terminated for PS2 drivers
    strcpy(game_dir, "cdrom0:\\ROMS\\");
    strcpy(cache_dir, "mc1:/");
}

int emu_start() {
    printf("[EMU] Initializing Globals...\n");
    emu_init_globals();

    // Force the menu loop immediately
    printf("[EMU] Entering Menu Loop (Forced)\n");
    return menu_loop();
}

void emu_exit_to_menu() {
    rom_unload();
    emu_status = EMU_MENU;
    printf("[EMU] Returned to Menu.\n");
}
