#include "gba_interrupt.h"
#include "gba_video.h"
#include "subsys/gfx.h"
#include "subsys/input.h"
#include <runtime/runtime.h>
#include <non_canon/err.h>
#include <stdint.h>
#include <subsys/subsys.h>
#include <subsys/assets.h>
#include <util/lowlevel.h>


extern char health_gfx;
void* text_graphic;

//original addr: $0805a568
void hns_load_graphics(){
    void* loaded;

    loaded = index_asset_tbl(&health_gfx, 0);
    lz777_uncomp_vram(loaded,(void*)0x6008000);
    loaded = index_asset_tbl(&health_gfx, 2);
    lz777_uncomp_vram(loaded,(void*)0x6000000);
    loaded = index_asset_tbl(&health_gfx, 3);
    lz777_uncomp_vram(loaded,(void*)0x6010000);

    text_graphic = index_asset_tbl(&health_gfx, 4);

    void* palette_data = index_asset_tbl(&health_gfx, 1);
    copy_pal_staging(&glob_gfx_staging,palette_data,0,0x20);
    copy_pal_staging(&glob_gfx_staging,palette_data,0x10,0x20);
    //TODO: figure out what these copy calls do
    glob_gfx_regs.dispcnt = 0x140;
    glob_gfx_regs.bg0cnt = 8;
    glob_gfx_regs.bg1cnt = 0;
    glob_gfx_regs.bg2cnt = 0;
    glob_gfx_regs.bg3cnt = 0;
    glob_gfx_regs.bldalpha = 0;
    glob_gfx_regs.bldy = 0;
}

//original addr: $0805a800
void hns_text_draw(){
    void* tmp = gfx_get_arr_data_ptr(text_graphic);
    uint16_t* obj = gfx_get_arr_data_obj(tmp, 0);
    gfx_display_complex_obj(obj, 0, 0, 0, 0x78, 0xa0);
}

uint16_t text_fluc_dir;
uint16_t text_alpha;

//original addr: $0805a778
void hns_text_update(uint16_t stage){
    glob_gfx_regs.dispcnt |= 0x1000;
    hns_text_draw();
    glob_gfx_regs.bldcnt = 0x150;
    glob_gfx_regs.bldalpha = text_alpha << 8 | 0x10;
    glob_gfx_regs.bldy = 0;
    if(stage != 2){
        int16_t delta;
        if(text_fluc_dir == 0){
            if (text_alpha == 0){
                text_fluc_dir = 1;
                glob_gfx_regs.bldcnt = 0x150;
                glob_gfx_regs.bldy = 0;
                return;
            }
            delta = 1;
        } else {
            if (text_alpha == 0x10){
                text_fluc_dir = 0;
                glob_gfx_regs.bldcnt = 0x150;
                glob_gfx_regs.bldy = 0;
                return;
            }
            delta = -1;
        }
        text_alpha += delta;
    }
}

//original addr: $0805a638
void hns_loop(){
    uint16_t stage = 0;
    uint32_t frame_count = 0;
    //TODO: investigate this mystery struct
    bool should_exit = false;

    text_alpha = 0x10;
    text_fluc_dir = 0;

    //TODO: figure out these mystery sturct functions
    while (true){
        if(should_exit){
            return;
        }
        if (stage == 1){
            if (frame_count > 0xe0f){
                stage = 2;
                frame_count = 0;
                //TODO: figure out these mystery sturct functions
            }
        } else if (stage == 0){
            stage = 1;
            frame_count = 0;
        } else if (stage == 2){
            should_exit = 1;
        }

        gfx_staging_clear(&glob_gfx_staging);
        inp_update(&glob_inp_state);

        //weird CF that I'm lazy to investigate
        if (stage == 2){
            draw_text:
               hns_text_update(stage);
            dont_draw_text:
            if(stage == 1 && frame_count > 0x59 && glob_inp_state.down != 0){
                stage = 2;
                frame_count = 0;
                //TODO: figure out these mystery sturct functions
            }
        } else if (stage == 1){
            if (frame_count > 0x59)
                goto draw_text;

            goto dont_draw_text;
        }

        wait_for_vblank();
        gfx_commit();
        frame_count++;
    }
}

//original addr: $0805a310
void hns_loop_wrapper(){
    REG_IE |= 1;
    REG_DISPSTAT |= 8;
    REG_IME = 1;
    hns_loop();
}

//original addr: $0805a2f4
void hns_main(){
    hns_load_graphics();
    //TODO: whatever the hell those mystery flags do
    hns_loop_wrapper();
}

//original addr: $0805a2dc
void hns_body(){
    //TODO: implement load_hns_jmptbl
    init_subsystems();
    //I refuse to implement $0805a378 because it looks useless
    hns_main();
}

//original addr: $080007b0
void rt_mode_hns(){
    //TODO: implement clear_sound
    hns_body();
    //TODO: implement mystery sound op($080037d8)
    runtime.mode = MODE_FT_DISCLAIMER;

}