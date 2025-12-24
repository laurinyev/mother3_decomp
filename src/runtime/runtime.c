#include "runtime.h"
#include <non_canon/err.h>
#include <non_canon/int.h>
#include <stdbool.h>

//note: while you may call this "main_loop", 
//that is incorrect as this is not the main frame loop, 
//instead the mode functions only return when they try to switch modes
//original addr: $080002e0
void runtime_dispatch(){
	do {
		switch(runtime.mode){
			case MODE_NEW_GAME:
                rt_mode_naming();
                rt_mode_gameplay();
                //TODO: if (main_menu_outcome != 6) mode_menu();
                goto start_game;
			case MODE_UNUSED:
				rt_mode_unused();
                rt_mode_gameplay();
                start_game:
                runtime.mode = MODE_PLAY_FROM_NEW_GAME;
                //gameplay_mode = MODE_NEW_GAME;
                break;
			case MODE_PLAY_FROM_NEW_GAME:    //fallthru
			case MODE_PLAY_FROM_CONTINUE: 	 //fallthru
			case MODE_PLAY_FROM_RESPAWN: 	 //fallthru
			case MODE_PLAY_FROM_BATTLE_WIN:  //fallthru
			case MODE_PLAY_FROM_MENU: 	 	 //fallthru
			case MODE_PLAY_FROM_NEW_CHAPTER: //fallthru
				rt_mode_gameplay();
                break;
			case MODE_BATTLE:
				rt_mode_battle();
                break;
			case MODE_FULLSCREEN_MENU:
				rt_mode_menu();
                break;
			case MODE_MAIN_MENU:
				rt_mode_main_menu();
                break;
			case MODE_CREDITS:
				rt_mode_credits();
                break;
			case MODE_SNDPLYR:
				rt_mode_sndplyr();
                break;
			case MODE_HNS:
				rt_mode_hns();
                break;
			case MODE_FT_DISCLAIMER:
				rt_mode_ft_disclaimer();
                //TODO: little bit of unknown variable setting here
			default:
				break;
		}
		install_ivt1();
	} while(true);
}