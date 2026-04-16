/******************************************************************************
    cps2.c
    CPS2 emulation core - Hardcoded for AVSPU
******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <unistd.h>      // Required for chdir
#include "cps2.h"

// External declarations to match the rest of the engine
extern char launchDir[MAX_PATH];
extern char game_dir[MAX_PATH];
extern char cache_dir[MAX_PATH];

void cps2_main(void)
{
    // --- 1. FORCE PATHS IMMEDIATELY ---
    // This overrides any 'host:/' detection from the loader
    strcpy(launchDir, "cdrom0:\\");
    strcpy(game_dir, "cdrom0:\\ROMS");
    strcpy(cache_dir, "mc1:");
    chdir("cdrom0:\\"); 

    // --- 2. IOP & RPC RESET ---
    SifInitRpc(0);
    SifLoadModule("rom0:LIBSD", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);   // Required for mc1:
    SifLoadModule("rom0:MCSERV", 0, NULL);  // Required for mc1:
    
    // --- 3. MODULE LOADING ---
    int aud_ret = SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    int usbd_ret = SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    int kbd_ret = SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    printf("[PS2] Launch: %s | Game: %s | Cache: %s\n", launchDir, game_dir, cache_dir);

    // Give the IOP time to settle
    for(int i = 0; i < 6000000; i++) { __asm__("nop"); }

    strcpy(game_name, "AVSPU");

    // --- 4. THREAD MANAGEMENT ---
    ChangeThreadPriority(GetThreadId(), 72);

    Loop = LOOP_RESET;
    while (Loop >= LOOP_RESTART)
    {
        Loop = LOOP_EXEC;
        fatal_error = 0;
        video_clear_screen();

        if (memory_init())
        {
            if (sound_init())
            {
                input_init();
                // cps2_init is likely where rominfo.cps2 is opened
                if (cps2_init()) 
                {
                    cps2_run();
                }
                cps2_exit();
                input_shutdown();
            }
            sound_exit();
        }
        memory_shutdown();
    }
}
