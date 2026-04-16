/******************************************************************************
    cps2.c
    CPS2 emulation core - Hardcoded for AVSPU
******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include "cps2.h"

void cps2_main(void)
{
    // --- 1. IOP & RPC RESET ---
    SifInitRpc(0);
    
    // Load fundamental sound library from PS2 ROM
    SifLoadModule("rom0:LIBSD", 0, NULL);
    
    // --- 2. MODULE LOADING ---
    // audsrv MUST be loaded before sound_init() or SifBindRpc will fail
    int aud_ret = SifLoadModule("cdrom0:\\AUDSRV.IRX;1", 0, NULL);
    int usbd_ret = SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    int kbd_ret = SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    // Logging to PCSX2 console for debugging
    printf("[PS2] AUDSRV Load: %d, USBD: %d, KBD: %d\n", aud_ret, usbd_ret, kbd_ret);

    // Give the IOP time to settle and register the RPC services
    int i;
    for(i = 0; i < 6000000; i++) { __asm__("nop"); }

    strcpy(game_name, "AVSPU");

    // --- 3. THREAD MANAGEMENT ---
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);

    Loop = LOOP_RESET;
    while (Loop >= LOOP_RESTART)
    {
        Loop = LOOP_EXEC;
        fatal_error = 0;
        video_clear_screen();

        if (memory_init())
        {
            // This should now succeed as AUDSRV.IRX is live on the IOP
            if (sound_init())
            {
                input_init();

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
