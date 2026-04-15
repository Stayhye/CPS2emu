/******************************************************************************
    cps2.c
    CPS2 emulation core - Hardcoded for Alien vs. Predator (avspu)
******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include "cps2.h"

/******************************************************************************
    Local function
******************************************************************************/

/*--------------------------------------------------------
    CPS2 emulation initialize
--------------------------------------------------------*/
static int cps2_init(void)
{
#ifdef HISCORE
    hs_clear();
    if (option_hiscore) hs_open();
#endif
    // This will now look for the "avspu" driver definition
    cps2_driver_init();

    return cps2_video_init();
}

/*--------------------------------------------------------
    CPS2 emulation reset
--------------------------------------------------------*/
static void cps2_reset(void)
{
    cps2_driver_reset();
    cps2_video_reset();

    timer_reset();
    input_reset();
    sound_reset();
    sound_volume(option_sound_volume);
    
    msg_screen_clear();
    autoframeskip_reset();

    Loop = LOOP_EXEC;
}

/*--------------------------------------------------------
    CPS2 emulation shutdown and exit
--------------------------------------------------------*/
static void cps2_exit(void)
{
    video_clear_screen();
    msg_screen_init();

    msg_printf("System shutdown.\n");

#ifdef HISCORE
    hs_close();
#endif
    cps2_video_exit();
    cps2_driver_exit();
    save_gamecfg(game_name);

    msg_printf("Done.\n");
}

/*--------------------------------------------------------
    CPS2 emulation start
--------------------------------------------------------*/
static void cps2_run(void)
{
    while (Loop >= LOOP_RESET)
    {
        cps2_reset();

        while (Loop == LOOP_EXEC)
        {
            timer_update_cpu();
#ifdef HISCORE
            if (option_hiscore) hs_update();
#endif
            update_inputport();
            update_screen();
        }

        video_clear_screen();
        sound_mute(1);
    }
}

/******************************************************************************
    Global function
******************************************************************************/

/*--------------------------------------------------------
    CPS2 emulation main
--------------------------------------------------------*/
void cps2_main(void)
{
    // --- PS2 HARDWARE BOOT SEQUENCE ---
    // 1. Set Thread Priority
    int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);

    // 2. Initialize SIF and Load Modules from ISO
    // This is mandatory to prevent the "Unknown device usbkbd" crash
    SifInitRpc(0);
    SifLoadModule("cdrom0:\\USBD.IRX;1", 0, NULL);
    SifLoadModule("cdrom0:\\USBKBD.IRX;1", 0, NULL);

    // 3. Force the game name to the US version of AvP
    // The emulator will now look for avspu.zip or extracted avspu files
    strcpy(game_name, "avspu");

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
