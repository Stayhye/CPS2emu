/******************************************************************************
    cps2.c
    CPS2 emulation core - Hardcoded for AVSPU
******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <unistd.h>      
#include <malloc.h>      
#include "cps2.h"

// External declarations for the global engine state
extern char launchDir[MAX_PATH];
extern char game_dir[MAX_PATH];
extern char cache_dir[MAX_PATH];
extern u8 *memory_region_gfx1;

void cps2_main(void)
{
    // --- 1. FORCE PATHS FOR HARDWARE ---
    // This stops the engine from guessing "host:"
    strcpy(launchDir, "cdrom0:\\");
    strcpy(game_dir, "cdrom0:\\ROMS");
    strcpy(cache_dir, "mc1:");
    
    // --- 2. IOP & RPC RESET ---
    SifInitRpc(0);
    while(!SifIopReset(NULL, 0));
    while(!SifIopSync());
    SifInitRpc(0);
    SifLoadFileInit();

    // Standard BIOS modules
    SifLoadModule("rom0:LIBSD", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);   
    SifLoadModule("rom0:MCSERV", 0, NULL);  
    
    // --- 3. MODULE LOADING ---
    printf("[PS2] Loading IRX modules...\n");
    SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    printf("[PS2] Config: ROMS=%s | CACHE=%s\n", game_dir, cache_dir);

    strcpy(game_name, "AVSPU");

    // --- 4. THREAD MANAGEMENT ---
    ChangeThreadPriority(GetThreadId(), 72);

    Loop = LOOP_RESET;
    while (Loop >= LOOP_RESTART)
    {
        Loop = LOOP_EXEC;
        fatal_error = 0;

        if (memory_init())
        {
            if (sound_init())
            {
                input_init();
                
                printf("[PS2] Running cps2_init...\n");
                
                if (cps2_init()) 
                {
                    // Check if memory actually exists before jumping in
                    if (memory_region_gfx1 == NULL) {
                        printf("CPS2EMU FATAL: GFX Memory is NULL. AvP is too large for RAM.\n");
                        // Infinite loop to stop the TLB crash
                        while(1) { iVblank(); } 
                    } else {
                        printf("[PS2] Memory allocated at 0x%08X. Booting...\n", (u32)memory_region_gfx1);
                        cps2_run();
                    }
                } 
                else 
                {
                    // This is likely where it fails. 
                    printf("CPS2EMU FATAL: cps2_init failed!\n");
                    printf("Ensure ROMINFO.CPS2 is in the ROOT of your ISO.\n");
                    // Freeze here so we can read the log
                    while(1) { iVblank(); }
                }

                cps2_exit();
                input_shutdown();
            }
            sound_exit();
        }
        memory_shutdown();
    }
}
