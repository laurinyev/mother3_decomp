#include "gfx.h"
#include "gba_dma.h"
#include "gba_systemcalls.h"
#include "gba_video.h"
#include <stdint.h>
#include <util/lowlevel.h>

gfx_regs_t glob_gfx_regs;
char glob_gfx_staging[0x2B00]; //TODO: make this gigantic struct

//original address: $080013d0
void init_gfx_regs(gfx_regs_t* regs){
    //TODO: full implementation of the gfx regs
    regs->bldcnt = 0;
    regs->dispcnt = 0;
    //regs->bldalpha = 0; the original doesnt initialize these (????)
    //regs->bldy = 0;
    for(int i = 0; i < 4;i++){
        (&regs->bg0cnt)[i] = 0;
    }
}

//original address: $08001454
void clear_gfx_staging(void* staging){
  *(uint16_t *)(staging + 0x2c46) = 0;
  *(uint16_t *)(staging + 0x2c44) = *(uint16_t*)(staging + 0x2c46);
  *(uint16_t *)(staging + 0x2c42) = *(uint16_t*)(staging + 0x2c44);
  *(uint16_t *)(staging + 0x2c40) = *(uint16_t*)(staging + 0x2c42);
  do_dma_clear(staging,0x800);
  do_dma_clear(staging + 0x800,0x800);
  do_dma_clear(staging + 0x1000,0x800);
  do_dma_clear(staging + 0x1800,0x800);
  do_dma_clear(staging + 0x2700,0x400);
  *(uint16_t *)(staging + 0x2c48) = 0;
  *(uint16_t *)(staging + 0x2c4a) = 0;
  *(uint8_t *)((int)staging + 0x2c4c) = 0;
  *(uint8_t *)((int)staging + 0x2c4d) = 0;
  *(uint8_t *)((int)staging + 0x2c4e) = 0;
  *(uint16_t *)((int)staging + 0x2700) =
       (uint16_t)*(uint8_t *)((int)staging + 0x2c4d) << 5 | (uint16_t)*(uint8_t *)((int)staging + 0x2c4c) |
       (uint16_t)*(uint8_t *)((int)staging + 0x2c4e) << 10;
}

//original addr: $0805a4bc
void gfx_regs_commit(gfx_regs_t* regs){
    //TODO: investigate mystery condition for this block
    REG_DISPCNT = regs->dispcnt;
    REG_BG0CNT = regs->bg0cnt;
    REG_BG1CNT = regs->bg1cnt;
    REG_BG2CNT = regs->bg2cnt;
    REG_BG3CNT = regs->bg3cnt;

    REG_BLDCNT = regs->bldcnt;
    REG_BLDALPHA = regs->bldalpha;
    REG_BLDY = regs->bldy;
}

//original addr: $
void apply_staging_oam(void* staging){
    DMA3COPY(staging + 0x2000, 0x7000000, 0x80000200);
    while(REG_DMA3CNT & 0x80000000);
}


void apply_staging_pal(void* staging){
    DMA3COPY(staging + 0x2700, 0x5000000, 0x80000200);
    while(REG_DMA3CNT & 0x80000000);
}

//original addr: $0805a508
void apply_staging(void* staging){
    apply_staging_oam(staging);
    apply_staging_pal(staging);
}

//original addr: $08000e5c
void gfx_staging_clear(void* staging){
    for(int i = 0; i < 0x80; i++){
        uint8_t* p = staging + 0x2000 + i*8;

        p[0]  = 0xa0;
        p[3] &= 0x3e;
        p[1] &= 0x3f;
    }
    for(int i = 0; i < 0x20; i++){
        uint16_t* p = staging + 0x2500 + i*8;
        p[0]  = 0;
        p[1]  = 0;
        p[2]  = 0;
        p[3]  = 0;
    }
    *(uint16_t*)(staging + 0x2c48) = 0;
    *(uint16_t*)(staging + 0x2c4a) = 0;

}

//original address: $08002c4c
void* gfx_get_arr_data_ptr(void* ptr){
    return ptr + ((uint16_t*)ptr)[1];
}

//original address: $08002abc
void* gfx_get_arr_data_obj(uint16_t* ptr, uint16_t num){
    uint16_t* ent = ptr + 1;
    uint16_t* ent2 = 0;
    uint16_t i = 0;

    do {
        ent2 = ent + (*ent * 3) + 1;
        ent = ent2 + (*ent2 * 3) + 1;
        i = i + 1;
    } while(i <= num);

    return (void*)ent2;
}

//original address: $08000f04
void* gfx_reserve_oam_sprites(void* staging, uint16_t num_sprites){
    uint16_t* num_spr = (uint16_t*)((uint8_t*)staging + 0x2c48);
    uint8_t* first_spr = staging + 0x2000 + (*num_spr * 8);
    uint8_t* cur_spr = first_spr;
    if (num_sprites != 0){
        for(uint16_t i = 0; i < num_sprites; i++){
            cur_spr[5] &= 0xf3 | 4;
            cur_spr[1] &= 0xc0;
            cur_spr[3] &= 0xc1;
            ((uint16_t*)cur_spr)[3] = 0;

            (*num_spr)++;
            cur_spr += 8;
        }
    }
    return first_spr;
}

//original address: $0805a844
void gfx_display_complex_obj(uint16_t* arr_data, int16_t tile_base, uint32_t pal_num, uint32_t priority, int16_t base_x, int16_t base_y){
    uint16_t* dst = gfx_reserve_oam_sprites(glob_gfx_staging, *arr_data);
    uint8_t* src = (uint8_t*)(arr_data + 1);

    if(*arr_data != 0){
        for(uint16_t i = 0; i < *arr_data; i++){
            dst[0] = ((*src + base_y) & 0xffU) | (src[1] << 8);
            dst[1] = (base_x + (((uint16_t*)src)[1] & 0x1ff)) | ((src[3] >> 1) << 9);
            dst[2] = (tile_base + (((uint16_t*)src)[2] & 0x3ff)) | ((priority & 0xff) << 10) | (pal_num << 0xc);
            dst += 4;
            src += 6;
        }
    }
}

//original address: $0800160c
void copy_pal_staging(void* staging, void* src, uint32_t off, uint32_t size){
    CpuFastSet(src,staging + 0x2700 + (off << 5) , size >> 2);
}

//original addr: $08001960
void wait_for_vblank(){
    VBlankIntrWait();
}

//original addr: $0805a488
void gfx_commit(){
    //TODO: figure out all the mysteries
    gfx_regs_commit(&glob_gfx_regs);
    apply_staging(&glob_gfx_staging);
}
