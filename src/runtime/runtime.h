#ifndef __RUNTIME_H__
#define __RUNTIME_H__

#include <stdint.h>

typedef __attribute__((mode(QI))) enum {
    MODE_NEW_GAME,
    MODE_UNUSED,
    MODE_PLAY_FROM_NEW_GAME,
    MODE_PLAY_FROM_CONTINUE,
    MODE_PLAY_FROM_RESPAWN,
    MODE_PLAY_FROM_BATTLE_WIN,
    MODE_PLAY_FROM_MENU,
    MODE_PLAY_FROM_NEW_CHAPTER,
    MODE_BATTLE,
    MODE_FULLSCREEN_MENU,
    MODE_MAIN_MENU,
    MODE_CREDITS,
    MODE_SNDPLYR,
    MODE_HNS,
    MODE_FT_DISCLAIMER
} runtime_mode_t;

typedef struct{
    runtime_mode_t mode;
    uint8_t flags;
} runtime_t;

void rt_mode_naming();
void rt_mode_unused();
void rt_mode_gameplay();
void rt_mode_battle();
void rt_mode_menu();
void rt_mode_main_menu();
void rt_mode_credits();
void rt_mode_sndplyr();
void rt_mode_hns();
void rt_mode_ft_disclaimer();

void runtime_dispatch();

extern runtime_t runtime;

#endif // __RUNTIME_H__