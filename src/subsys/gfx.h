#ifndef __GFX_H__
#define __GFX_H__

#include <stdint.h>

typedef struct {
    uint16_t bldcnt;
    uint16_t bldalpha;
    uint16_t bldy;
    uint16_t dispcnt;
    uint16_t bg0cnt;
    uint16_t bg1cnt;
    uint16_t bg2cnt;
    uint16_t bg3cnt;
} gfx_regs_t;

extern gfx_regs_t glob_gfx_regs;
extern char glob_gfx_staging[0x2B00]; //TODO: make this gigantic struct

void init_gfx_regs(gfx_regs_t* regs);
void clear_gfx_staging(void* staging);
void copy_pal_staging(void* staging, void* src, uint32_t off, uint32_t size);

void gfx_staging_clear(void* staging);

void* gfx_get_arr_data_ptr(void* ptr);
void* gfx_get_arr_data_obj(uint16_t* ptr, uint16_t num);

void* gfx_reserve_oam_sprites(void* staging, uint16_t num_srites);
void gfx_display_complex_obj(uint16_t* arr_data, int16_t tile_base, uint32_t pal_num, uint32_t priority, int16_t base_x, int16_t base_y);


void wait_for_vblank();
void gfx_commit();

#endif // __GFX_H__