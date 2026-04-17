/******************************************************************************
    cps2.c
    Core CPS2 Engine Entry
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "emumain.h"
#include "common.h"
#include "cps2.h"

extern u8 *memory_region_gfx1;
extern u32 memory_length_gfx1;

/* The PS2 has 32MB RAM. Alien vs Predator is 23MB.
   We MUST use a streaming cache window of 8MB to avoid TLB Miss.
*/
#define SAFE_GFX_WINDOW 0x800000 

int cps2_main() {
    printf("[CPS2] Engine Booting...\n");

    // Clear hardware registers
    emu_status = EMU_MENU;

    // Allocate the fixed GFX window (8MB)
    // This prevents the "Upper memory full" error for the 23MB AvP ROM.
    if (memory_region_gfx1 == NULL) {
        memory_length_gfx1 = SAFE_GFX_WINDOW;
        memory_region_gfx1 = (u8 *)memalign(64, memory_length_gfx1);
    }

    if (!memory_region_gfx1) {
        printf("[CPS2] FATAL: Memory allocation failed!\n");
        return -1;
    }

    // Initialize the CPS2 hardware components
    if (cps2_hw_init() < 0) {
        printf("[CPS2] Hardware init failed!\n");
        return -1;
    }

    // Start the Menu Loop
    // We do NOT call rom_load() here.
    while (1) {
        if (emu_status == EMU_MENU) {
            int selected = menu_loop();
            if (selected >= 0) {
                // Only now do we try to load the ROM
                printf("[CPS2] Loading Selected Game...\n");
                if (rom_load(game_list[selected].name) == 0) {
                    emu_status = EMU_RUNNING;
                }
            }
        } else if (emu_status == EMU_RUNNING) {
            cps2_execute_frame();
            
            // Check for exit trigger
            if (emu_check_exit()) {
                emu_status = EMU_MENU;
                rom_unload();
            }
        }
    }

    return 0;
}
