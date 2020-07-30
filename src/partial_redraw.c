#include <stdbool.h>
#include <graphx.h>
#undef NDEBUG
#include <debug.h>
#include "partial_redraw.h"
#include "profiler.h"

bool pdraw_current_buffer = 0;
pdraw_sprite_t pdraw_sprites[2][32];
uint8_t pdraw_num_sprites[2] = {0, 0};

pdraw_sprite_t *pdraw_RectRegion(uint24_t x, uint8_t y, uint8_t width, uint8_t height) {
    profiler_add(store_bg);
    gfx_sprite_t *bg_sprite = gfx_MallocSprite(width, height);
    if(bg_sprite == NULL) {
        dbg_sprintf(dbgout, "Failed to allocate memory for redraw sprite\n");
        profiler_end(store_bg);
        return NULL;
    }

    gfx_GetSprite(bg_sprite, x, y);
    pdraw_sprite_t *psprite = &pdraw_sprites[pdraw_current_buffer][(pdraw_num_sprites[pdraw_current_buffer])++];
    psprite->sprite = bg_sprite;
    psprite->x = x;
    psprite->y = y;

    profiler_end(store_bg);
    return psprite;
}

void pdraw_Sprite_NoClip(gfx_sprite_t *sprite, uint24_t x, uint8_t y) {
    if(pdraw_RectRegion(x, y, sprite->width, sprite->height))
        gfx_Sprite_NoClip(sprite, x, y);
}

void pdraw_TransparentSprite_NoClip(gfx_sprite_t *sprite, uint24_t x, uint8_t y) {
    if(pdraw_RectRegion(x, y, sprite->width, sprite->height))
        gfx_TransparentSprite_NoClip(sprite, x, y);
}

void pdraw_RemoveSprites(void) {
    pdraw_current_buffer = !pdraw_current_buffer;
    for(int8_t i = pdraw_num_sprites[pdraw_current_buffer] - 1; i >= 0; i--) {
        pdraw_sprite_t *psprite = &pdraw_sprites[pdraw_current_buffer][i];
        gfx_Sprite_NoClip(psprite->sprite, psprite->x, psprite->y);
        free(psprite->sprite);
    }
    pdraw_num_sprites[pdraw_current_buffer] = 0;
}

void pdraw_FreeAll(void) {
    for(uint8_t buf = 0; buf < 2; buf++) {
        for(uint8_t i = 0; i < pdraw_num_sprites[buf]; i++) {
            free(pdraw_sprites[buf][i].sprite);
        }
        pdraw_num_sprites[buf] = 0;
    }
}