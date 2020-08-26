#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <stdlib.h>
#include <string.h>

#include <graphx.h>


#include "level.h"
#include "graphics.h"
#include "gfx/gfx.h"
#include "util.h"
#include "globals.h"
#include "profiler.h"
#include "partial_redraw.h"
#include "dynamic_sprites.h"
#include "gui.h"
#include "gfx/enemy_pal.h"

uint8_t tilemap[TILEMAP_HEIGHT][TILEMAP_WIDTH];
// For each tilemap tile, the level Y of the block that it's representing
uint8_t depthmap[TILEMAP_HEIGHT][TILEMAP_WIDTH];

bool needs_redraw;

void repalettize_sprite(gfx_sprite_t *out, const gfx_sprite_t *in, const uint8_t *map) {
    out->width = in->width;
    out->height = in->height;

    size_t size = in->width * in->height;

    for(uint24_t i = 0; i < size; i++) {
        out->data[i] = map[in->data[i]];
    }
}

bool init_tank_sprites(tank_type_t type) {
    // return if sprites were already initialized from the last level
    if(tank_bases[type][0]) return true;

    spriteset_t *spriteset = malloc(sizeof(spriteset_t));
    if(!spriteset) return false;

    tank_bases[type][0] = (gfx_sprite_t *) &spriteset->base_0_data;
    tank_bases[type][1] = (gfx_sprite_t *) &spriteset->base_1_data;
    tank_bases[type][2] = (gfx_sprite_t *) &spriteset->base_2_data;
    tank_bases[type][3] = (gfx_sprite_t *) &spriteset->base_3_data;
    tank_bases[type][4] = (gfx_sprite_t *) &spriteset->base_4_data;
    tank_bases[type][5] = (gfx_sprite_t *) &spriteset->base_5_data;
    tank_bases[type][6] = (gfx_sprite_t *) &spriteset->base_6_data;
    tank_bases[type][7] = (gfx_sprite_t *) &spriteset->base_7_data;
    tank_bases[type][8] = (gfx_sprite_t *) &spriteset->base_8_data;
    tank_bases[type][9] = (gfx_sprite_t *) &spriteset->base_9_data;
    tank_bases[type][10] = (gfx_sprite_t *) &spriteset->base_10_data;
    tank_bases[type][11] = (gfx_sprite_t *) &spriteset->base_11_data;
    tank_bases[type][12] = (gfx_sprite_t *) &spriteset->base_12_data;
    tank_bases[type][13] = (gfx_sprite_t *) &spriteset->base_13_data;
    tank_bases[type][14] = (gfx_sprite_t *) &spriteset->base_14_data;
    tank_bases[type][15] = (gfx_sprite_t *) &spriteset->base_15_data;

    tank_turrets[type][0] = (gfx_sprite_t *) &spriteset->turret_0_data;
    tank_turrets[type][1] = (gfx_sprite_t *) &spriteset->turret_1_data;
    tank_turrets[type][2] = (gfx_sprite_t *) &spriteset->turret_2_data;
    tank_turrets[type][3] = (gfx_sprite_t *) &spriteset->turret_3_data;
    tank_turrets[type][4] = (gfx_sprite_t *) &spriteset->turret_4_data;
    tank_turrets[type][5] = (gfx_sprite_t *) &spriteset->turret_5_data;
    tank_turrets[type][6] = (gfx_sprite_t *) &spriteset->turret_6_data;
    tank_turrets[type][7] = (gfx_sprite_t *) &spriteset->turret_7_data;
    tank_turrets[type][8] = (gfx_sprite_t *) &spriteset->turret_8_data;
    tank_turrets[type][9] = (gfx_sprite_t *) &spriteset->turret_9_data;
    tank_turrets[type][10] = (gfx_sprite_t *) &spriteset->turret_10_data;
    tank_turrets[type][11] = (gfx_sprite_t *) &spriteset->turret_11_data;
    tank_turrets[type][12] = (gfx_sprite_t *) &spriteset->turret_12_data;
    tank_turrets[type][13] = (gfx_sprite_t *) &spriteset->turret_13_data;
    tank_turrets[type][14] = (gfx_sprite_t *) &spriteset->turret_14_data;
    tank_turrets[type][15] = (gfx_sprite_t *) &spriteset->turret_15_data;

    uint8_t palette_table[NUM_NON_DYNAMIC_COLORS + NUM_DYNAMIC_COLORS] = {0, COL_ENEMY_TANK_WOOD_1, COL_ENEMY_TANK_WOOD_2, COL_ENEMY_TANK_WOOD_3};
    for(uint8_t i = 0; i < NUM_DYNAMIC_COLORS; i++)
        palette_table[NUM_NON_DYNAMIC_COLORS + i] = 256 - NUM_DYNAMIC_COLORS * NUM_TANK_TYPES + NUM_DYNAMIC_COLORS * type + i;

    for(uint8_t rot = 0; rot < 9; rot++) {
        repalettize_sprite(tank_bases[type][rot], enemy_bases_unconv[rot], palette_table);
        repalettize_sprite(tank_turrets[type][rot], enemy_turrets_unconv[rot], palette_table);
    }
    for(uint8_t i = 1; i < 8; i++) {
        gfx_FlipSpriteY(tank_bases[type][i], tank_bases[type][16 - i]);
        gfx_FlipSpriteY(tank_turrets[type][i], tank_turrets[type][16 - i]);
    }
    return true;
}

void free_tank_sprites(tank_type_t type) {
    free(tank_bases[type][0]);
    memset(tank_bases[type], 0, sizeof(tank_bases[type]));
}

// this is supposed to go in dynamic_sprites.c, but a weird (linker?) issue causes this array to be filled with NULL if I do that
gfx_sprite_t *const enemy_bases_unconv[9] = {en_base_0, en_base_1, en_base_2, en_base_3, en_base_4, en_base_5,
                                             en_base_6, en_base_7, en_base_8};

gfx_sprite_t *const enemy_turrets_unconv[9] = {en_turret_0, en_turret_1, en_turret_2, en_turret_3, en_turret_4,
                                               en_turret_5, en_turret_6, en_turret_7, en_turret_8};

void init_graphics(void) {
    gfx_Begin(); //Set up draw bits
    gfx_SetPalette(palette, sizeof_palette, 0);
    gfx_SetPalette(enemy_pal, sizeof_enemy_pal, 256 - sizeof_enemy_pal / 2);
    gfx_SetDrawBuffer();
    gfx_SetTextFGColor(COL_RIB_SHADOW); // todo: verify

    for(uint8_t i = 1; i < 8; i++) {
        gfx_FlipSpriteY(tank_bases[PLAYER][i], tank_bases[PLAYER][16 - i]);
        gfx_FlipSpriteY(tank_turrets[PLAYER][i], tank_turrets[PLAYER][16 - i]);
        gfx_FlipSpriteY(shell_sprites[i], shell_sprites[16 - i]);
    }
}

void generate_bg_tilemap(void) {
    const bool bottom_is_tall[] = {0, 1, 0, 0, 1, 0, 0, 1};
    const bool bottom_is_alt[] = {0, 0, 1, 0, 0, 1, 0, 0};

    for(int8_t y = 0; y < TILEMAP_HEIGHT; y++) {
        uint8_t x;
        for(x = 0; x < TILEMAP_WIDTH; x++) {
            tilemap[y][x] = TS_NONE;
            depthmap[y][x] = y / 2 - TILEMAP_OFFSET;
        }
    }

    for(int8_t y = 0; y < LEVEL_SIZE_Y; y++) {
        uint8_t x;
        for(x = 0; x < TILEMAP_WIDTH; x++) {
            tile_t tile = tiles[y][x];
            uint8_t height = TILE_HEIGHT(tile);
            uint8_t type = TILE_TYPE(tile);
            bool tall = bottom_is_tall[height];
            bool alt = bottom_is_alt[height];
            uint8_t tm_y = 2 * y + TILEMAP_OFFSET;

            switch(type) {
                default:
                    break;
                case DESTROYED:
                    tilemap[tm_y][x] = TS_NONE;
                    tilemap[tm_y + 1][x] = TS_NONE;
                    depthmap[tm_y][x] = y;
                    depthmap[tm_y + 1][x] = y;
                    break;
                case HOLE:
                    tilemap[tm_y][x] = TS_HOLE_TOP;
                    tilemap[tm_y + 1][x] = TS_HOLE_BOT;
                    depthmap[tm_y][x] = y;
                    depthmap[tm_y + 1][x] = y;
                    break;
                case BLOCK: {
                    int8_t z;
                    if(!TILE_HEIGHT(tile)) {
                        tilemap[tm_y][x] = TS_NONE;
                        tilemap[tm_y + 1][x] = TS_NONE;
                        depthmap[tm_y][x] = y;
                        depthmap[tm_y + 1][x] = y;
                        break;
                    }

                    for(z = -1; z < height; z++) {
                        if(tall) {
                            if(alt) {
                                tilemap[tm_y - z - 1][x] = TS_SIDE_ALT_TOP;
                                tilemap[tm_y - z][x] = TS_SIDE_ALT_BOT;
                                depthmap[tm_y - z - 1][x] = y;
                                depthmap[tm_y - z][x] = y;
                            } else {
                                tilemap[tm_y - z - 1][x] = TS_SIDE_TOP;
                                tilemap[tm_y - z][x] = TS_SIDE_BOT;
                                depthmap[tm_y - z - 1][x] = y;
                                depthmap[tm_y - z][x] = y;
                            }
                            z++;
                        } else {
                            if(alt) {
                                tilemap[tm_y - z][x] = TS_SIDE_ALT_HALF;
                                depthmap[tm_y - z][x] = y;
                            } else {
                                tilemap[tm_y - z][x] = TS_SIDE_HALF;
                                depthmap[tm_y - z][x] = y;
                            }
                        }
                        alt = !alt;
                        tall = !tall;
                    }

                    tilemap[tm_y - z - 1][x] = TS_TOP_TOP;
                    tilemap[tm_y - z][x] = TS_TOP_BOT;
                    depthmap[tm_y - z - 1][x] = y;
                    depthmap[tm_y - z][x] = y;

                    break;
                }
                case DESTRUCTIBLE: {
                    int8_t z;
                    if(!TILE_HEIGHT(tile)) {
                        tilemap[tm_y][x] = TS_NONE;
                        tilemap[tm_y + 1][x] = TS_NONE;
                        depthmap[tm_y][x] = y;
                        depthmap[tm_y + 1][x] = y;
                        break;
                    }

                    for(z = -1; z < height; z++) {
                        if(tall) {
                            tilemap[tm_y - z - 1][x] = TS_DEST_SIDE_TOP;
                            tilemap[tm_y - z][x] = TS_DEST_SIDE_BOT;
                            depthmap[tm_y - z - 1][x] = y;
                            depthmap[tm_y - z][x] = y;
                            z++;
                        } else {
                            tilemap[tm_y - z][x] = TS_DEST_SIDE_HALF;
                            depthmap[tm_y - z][x] = y;
                        }
                        tall = !tall;
                    }

                    tilemap[tm_y - z - 1][x] = TS_DEST_TOP_TOP;
                    tilemap[tm_y - z][x] = TS_DEST_TOP_BOT;
                    depthmap[tm_y - z - 1][x] = y;
                    depthmap[tm_y - z][x] = y;

                    break;
                }
            }
        }
    }
}

const gfx_tilemap_t tilemap_config = {(uint8_t *) tilemap, tileset_tiles, HALF_TILE_PIXEL_HEIGHT, TILE_PIXEL_SIZE_X,
                                      TILEMAP_HEIGHT, TILEMAP_WIDTH, gfx_tile_no_pow2, gfx_tile_no_pow2, TILEMAP_HEIGHT,
                                      TILEMAP_WIDTH, TILEMAP_BASE_Y, SCREEN_X(0)};

void redraw_tile(uint8_t x, uint8_t y) {
    gfx_sprite_t *tile = tileset_tiles[tilemap[y][x]];
    uint24_t screen_x = SCREEN_X(TILE_SIZE * x);
    uint8_t screen_y = TILEMAP_BASE_Y + HALF_TILE_PIXEL_HEIGHT * y;
    gfx_Sprite(tile, screen_x, screen_y);
}

// Convert a screenspace coordinate to a redraw tile
uint8_t inline screen_to_tm_x(uint24_t screen_x) {
    int24_t dx = screen_x - SCREEN_X(0);
    return dx / SCREEN_DELTA_X(TILE_SIZE) - (dx < 0);
}

uint8_t inline screen_to_tm_y(uint24_t screen_y) {
    return (screen_y - TILEMAP_BASE_Y) / HALF_TILE_PIXEL_HEIGHT;
}

// todo: use sprites instead?
void draw_aim_dots(void) {
    profiler_start(aim_indicator);
    const uint8_t num_dots = 8;
    line_seg_t line;
    angle_t angle = tanks[0].barrel_rot;

    raycast(center_x(&tanks[0].phys), center_y(&tanks[0].phys), angle, &line);

    int24_t dx = (line.x2 - line.x1) / num_dots;
    int24_t dy = (line.y2 - line.y1) / num_dots;

    int24_t x = line.x1 + dx;
    int24_t y = line.y1 + dy;

    for(uint8_t dot = 0; dot < num_dots; dot++) {
        gfx_SetColor(COL_LIVES_TXT);
        pdraw_RectRegion(SCREEN_X(x) - AIM_INDICATOR_RADIUS, SCREEN_Y(y) - AIM_INDICATOR_RADIUS,
                         2 * AIM_INDICATOR_RADIUS + 1, 2 * AIM_INDICATOR_RADIUS + 1);
        gfx_FillCircle(SCREEN_X(x), SCREEN_Y(y), AIM_INDICATOR_RADIUS);
        x += dx;
        y += dy;
    }
    profiler_end(aim_indicator);
}

void render_obscured_object(gfx_sprite_t **sprites, const uint8_t *offsets_x, const uint8_t *offsets_y,
                            const physics_body_t *phys, uint8_t rotation) {
    profiler_add(render_obscured);
    uint24_t base_x = SCREEN_X(center_x(phys)) - SPRITE_OFFSET_X;
    uint8_t base_y = SCREEN_Y(center_y(phys)) - SPRITE_OFFSET_Y;
    uint8_t obj_y = COORD_TO_Y_TILE(phys->position_y); // -1 is to round down if exactly on the edge
    gfx_sprite_t *sprite = sprites[rotation];
    uint24_t sprite_x = base_x + offsets_x[rotation];
    uint8_t sprite_y = base_y + offsets_y[rotation];
    uint24_t sprite_end_x = sprite_x + sprite->width;
    uint8_t sprite_end_y = sprite_y + sprite->height;

    pdraw_TransparentSprite(sprite, sprite_x, sprite_y);

    profiler_add(redraw_tile);
    for(uint8_t tile_x = screen_to_tm_x(sprite_x); tile_x <= screen_to_tm_x(sprite_end_x); tile_x++) {
        for(uint8_t tile_y = screen_to_tm_y(sprite_y); tile_y <= screen_to_tm_y(sprite_end_y); tile_y++) {
            int8_t world_tile_y = depthmap[tile_y][tile_x];
            // todo: this really feels like a hackfix
            if(world_tile_y >= obj_y && tilemap[tile_y][tile_x] != TS_NONE && tilemap[tile_y][tile_x] != TS_HOLE_BOT &&
                tilemap[tile_y][tile_x] != TS_HOLE_TOP) {
                redraw_tile(tile_x, tile_y);
            }
        }
    }
    profiler_end(redraw_tile);
    profiler_end(render_obscured);
}

void render_shell(shell_t *shell) {
    if(shell->alive) {
        uint8_t sprite = shell->direction;
        render_obscured_object(shell_sprites, shell_x_offsets, shell_y_offsets, &shell->phys, sprite);
    }
}

void render_tank(tank_t *tank) {
    if(tank->alive) {
        uint8_t base_sprite = (((uint8_t) -((tank->tread_rot >> 16) - 64)) >> 3) & 0xF;
        uint8_t turret_sprite = ((uint8_t) -((tank->barrel_rot >> 16) - 64)) >> 4;

        // todo: a lot of this seems to be running twice as often as it needs to
        if(tank->type == PLAYER) {
            render_obscured_object(tank_bases[tank->type], pl_base_x_offsets, pl_base_y_offsets, &tank->phys,
                                   base_sprite);
            render_obscured_object(tank_turrets[tank->type], pl_turret_x_offsets, pl_turret_y_offsets, &tank->phys,
                                   turret_sprite);
        } else {
            render_obscured_object(tank_bases[tank->type], en_base_x_offsets, en_base_y_offsets, &tank->phys,
                                   base_sprite);
            render_obscured_object(tank_turrets[tank->type], en_turret_x_offsets, en_turret_y_offsets, &tank->phys,
                                   turret_sprite);
        }
    }

    //draw shell hitboxes until I can get sprites
    profiler_add(render_shells);
    for(int8_t j = max_shells[tank->type] - 1; j >= 0; j--) {
        shell_t *shell = &tank->shells[j];
        if(shell->alive) render_shell(shell);
    }
    profiler_end(render_shells);
    //draw mine hitboxes
    profiler_add(render_mines);
    for(int8_t j = max_mines[tank->type] - 1; j >= 0; j--) {
        mine_t *mine = &tank->mines[j];
        if(!mine->countdown) continue;
        gfx_SetColor(COL_RED);
        if(mine->alive) gfx_SetColor(COL_BLACK);
        render_physics_body(&mine->phys);
    }
    profiler_end(render_mines);
}

void render(level_t *level) {
    profiler_start(graphics);

    profiler_start(gfx_wait);
    gfx_Wait();
    profiler_end(gfx_wait);

    if(needs_redraw) {
        profiler_start(tilemap);
        generate_bg_tilemap();
        gfx_Tilemap(&tilemap_config, 0, 0);
        display_game_banner(game.mission + 1, game.lives);
        display_game_kill_counter();
        gfx_BlitBuffer();
        pdraw_FreeAll();
        update_game_kill_counter(game.total_kills, true);
        needs_redraw = false;
        profiler_end(tilemap);
    }

    update_game_kill_counter(game.total_kills, false);

    profiler_start(undraw);
    pdraw_RemoveSprites();
    profiler_end(undraw);

    profiler_start(render_tanks);
    // todo: z-sorting
    // restrict drawing to only the play area, to prevent the banners from being overwritten
    // todo: deal with this
    gfx_SetClipRegion(SCREEN_X(0), SCREEN_Y(-TILE_SIZE), SCREEN_X(LEVEL_SIZE_X * TILE_SIZE),
                      SCREEN_Y(LEVEL_SIZE_Y * TILE_SIZE - TILE_SIZE));
    for(uint8_t i = 0; i < level->num_tanks; i++) {
        render_tank(&tanks[i]);
    }
    gfx_SetClipRegion(0, 0, LCD_WIDTH, LCD_HEIGHT);
    profiler_end(render_tanks);

    // todo: move to GUI section?
    draw_aim_dots();

    profiler_start(swapdraw);
    gfx_SwapDraw();
    profiler_end(swapdraw);

    profiler_end(graphics);
}

void render_physics_body(physics_body_t *phys) {
    uint24_t x = SCREEN_X(phys->position_x);
    uint8_t y = SCREEN_Y(phys->position_y);
    uint8_t width = SCREEN_DELTA_X(phys->width);
    uint8_t height = SCREEN_DELTA_Y(phys->height);
    pdraw_RectRegion(x, y, width, height);
    gfx_Rectangle(x, y, width, height);
}

void draw_line(line_seg_t *ls) {
    gfx_Line(SCREEN_X(ls->x1), SCREEN_Y(ls->y1), SCREEN_X(ls->x2), SCREEN_Y(ls->y2));
}
