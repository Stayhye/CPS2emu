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
#include <malloc.h>      // Required for memory checks
#include "cps2.h"

extern char launchDir[MAX_PATH];
extern char game_dir[MAX_PATH];
extern char cache_dir[MAX_PATH];

// External pointer used by the engine for ROM data
extern u8 *memory_region_gfx1;

void cps2_main(void)
{
    // --- 1. FORCE PATHS ---
    strcpy(launchDir, "cdrom0:\\");
    strcpy(game_dir, "cdrom0:\\ROMS");
    strcpy(cache_dir, "mc1:");
    
    // --- 2. IOP & RPC RESET ---
    SifInitRpc(0);
    // Standard PS2 Module Init
    while(!SifIopReset(NULL, 0));
    while(!SifIopSync());
    SifInitRpc(0);
    SifLoadFileInit();

    SifLoadModule("rom0:LIBSD", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);   
    SifLoadModule("rom0:MCSERV", 0, NULL);  
    
    // --- 3. MODULE LOADING ---
    // Note: Added ;1 for ISO9660 compatibility
    SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    printf("[PS2] Paths Forced: ROMS=%s | CACHE=%s\n", game_dir, cache_dir);

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
                
                printf("[PS2] Initializing CPS2 Engine...\n");
                
                if (cps2_init()) 
                {
                    // --- SAFETY CHECK ---
                    // If cps2_init passed but the pointer is still NULL, 
                    // AvP (23.1MB) likely failed to allocate.
                    if (memory_region_gfx1 == NULL || (u32)memory_region_gfx1 < 0x100000) {
                        printf("CPS2EMU FATAL: GFX Memory not allocated! (Out of Memory)\n");
                        Loop = LOOP_EXIT;
                    } else {
                        printf("[PS2] Memory OK at 0x%08X. Starting game...\n", (u32)memory_region_gfx1);
                        cps2_run();
                    }
                } 
                else 
                {
                    printf("CPS2EMU FATAL: cps2_init failed (check ROMINFO.CPS2)\n");
                    // Wait so you can see the error in PCSX2 before it exits
                    for(int i = 0; i < 10000000; i++) { __asm__("nop"); }
                }

                cps2_exit();
                input_shutdown();
            }
            sound_exit();
        }
        memory_shutdown();
    }
}
