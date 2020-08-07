#ifndef H_GRAPHICS
#define H_GRAPHICS

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "collision.h"
#include "level.h"
#include "gfx/gfx.h"
#include "tank.h"

enum colors {
    COL_WHITE = 1,
    COL_BLACK = 2,
    COL_RED = 3,
    COL_BG = 4,
    COL_RHOM_1 = 5,
    COL_RHOM_2 = 6,
    COL_RIB_SHADOW = 7,
    COL_GOLD = 8,
    COL_TXT_SHADOW = 9,
    COL_LIVES_TXT = 10,
    COL_OLIVE_BANDS = 11,
    COL_ENEMY_TANK_WOOD_1 = 12,
    COL_ENEMY_TANK_WOOD_2 = 13
};

#define NUM_DYNAMIC_COLORS 5

enum tileset {
    TS_TOP_TOP,
    TS_SIDE_TOP,
    TS_SIDE_ALT_TOP,
    TS_SIDE_HALF,
    TS_DEST_TOP_TOP,
    TS_DEST_SIDE_TOP,
    TS_DEST_SIDE_HALF,
    TS_HOLE_TOP,
    TS_TOP_BOT,
    TS_SIDE_BOT,
    TS_SIDE_ALT_BOT,
    TS_SIDE_ALT_HALF,
    TS_DEST_TOP_BOT,
    TS_DEST_SIDE_BOT,
    TS_NONE,
    TS_HOLE_BOT
};

typedef struct {
    uint8_t base_0_data[en_base_0_size];
    uint8_t base_1_data[en_base_1_size];
    uint8_t base_2_data[en_base_2_size];
    uint8_t base_3_data[en_base_3_size];
    uint8_t base_4_data[en_base_4_size];
    uint8_t base_5_data[en_base_5_size];
    uint8_t base_6_data[en_base_6_size];
    uint8_t base_7_data[en_base_7_size];
    uint8_t base_8_data[en_base_8_size];
    uint8_t base_9_data[en_base_7_size];
    uint8_t base_10_data[en_base_6_size];
    uint8_t base_11_data[en_base_5_size];
    uint8_t base_12_data[en_base_4_size];
    uint8_t base_13_data[en_base_3_size];
    uint8_t base_14_data[en_base_2_size];
    uint8_t base_15_data[en_base_1_size];

    uint8_t turret_0_data[en_turret_0_size];
    uint8_t turret_1_data[en_turret_1_size];
    uint8_t turret_2_data[en_turret_2_size];
    uint8_t turret_3_data[en_turret_3_size];
    uint8_t turret_4_data[en_turret_4_size];
    uint8_t turret_5_data[en_turret_5_size];
    uint8_t turret_6_data[en_turret_6_size];
    uint8_t turret_7_data[en_turret_7_size];
    uint8_t turret_8_data[en_turret_8_size];
    uint8_t turret_9_data[en_turret_7_size];
    uint8_t turret_10_data[en_turret_6_size];
    uint8_t turret_11_data[en_turret_5_size];
    uint8_t turret_12_data[en_turret_4_size];
    uint8_t turret_13_data[en_turret_3_size];
    uint8_t turret_14_data[en_turret_2_size];
    uint8_t turret_15_data[en_turret_1_size];
} spriteset_t;

//Target frame rate
#define TARGET_FPS 30

#define TILE_PIXEL_SIZE_X 18
#define TILE_PIXEL_SIZE_Y 12

/* Calculate the screen-space distance for a given world-space distance */
#define SCREEN_DELTA_X(x) ((x) * TILE_PIXEL_SIZE_X / TILE_SIZE)
#define SCREEN_DELTA_Y(y) ((y) * TILE_PIXEL_SIZE_Y / TILE_SIZE)

#define HALF_TILE_PIXEL_HEIGHT (TILE_PIXEL_SIZE_Y / 2)

#define TILEMAP_OFFSET 4
#define TILEMAP_HEIGHT (2 * LEVEL_SIZE_Y + TILEMAP_OFFSET + 2)
#define TILEMAP_WIDTH (LEVEL_SIZE_X + 2)
#define TILEMAP_BASE_Y (SCREEN_Y(0) - HALF_TILE_PIXEL_HEIGHT * TILEMAP_OFFSET)

//Offset from sides of screen
#define MAP_OFFSET_X ((LCD_WIDTH  - SCREEN_DELTA_X(LEVEL_SIZE_X * TILE_SIZE)) / 2)
// Offset from top of the screen
#define MAP_OFFSET_Y 24

/* Calculate the screen-space position for a given world-space point */
#define SCREEN_X(x) (SCREEN_DELTA_X(x) + MAP_OFFSET_X)
#define SCREEN_Y(y) (SCREEN_DELTA_Y(y) + MAP_OFFSET_Y)

#define SPRITE_OFFSET_X 20
#define SPRITE_OFFSET_Y 22
#define SPRITE_SIZE_X 40
#define SPRITE_SIZE_Y 30

// todo: get an actual value?
#define AIM_INDICATOR_DOT_DISTANCE (2 * TILE_SIZE)
#define AIM_INDICATOR_RADIUS 2

void init_graphics(void);

void render(level_t *level); //Render tilemap, tanks, and UI during the game loop

void render_physics_body(physics_body_t *phys);

void draw_line(line_seg_t *ls);

void generate_bg_tilemap(void);

bool init_tank_sprites(tank_type_t type);

void free_tank_sprites(tank_type_t type);

extern bool needs_redraw; // set if the entire map should be redrawn

#endif
