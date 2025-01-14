#include "tank.h"

#include <new>

#include <debug.h>
#include <keypadc.h>

#include "../ai/ai.h"
#include "../util/profiler.h"
#include "../graphics/dynamic_sprites.h"
#include "../graphics/graphics.h"
#include "../graphics/partial_redraw.h"
#include "mine_detector.h"
#include "../graphics/tiles.h"
#include "../graphics/tank_sprite.h"
#include "../game.h"

const struct Tank::type_data Tank::types[NUM_TANK_TYPES] = {
        {5, 1, 2, (uint8_t)TANK_SPEED_HIGH},
        {1, 1, 0, (uint8_t)0},
        {1, 1, 0, (uint8_t)TANK_SPEED_SLOW},
        {1, 0, 0, (uint8_t)TANK_SPEED_SLOW},
        {1, 1, 4, (uint8_t)TANK_SPEED_HIGH},
        {3, 1, 0, (uint8_t)TANK_SPEED_NORMAL},
        {2, 2, 0, (uint8_t)0},
        {5, 1, 2, (uint8_t)TANK_SPEED_HIGH},
        {5, 1, 2, (uint8_t)TANK_SPEED_NORMAL},
        {2, 0, 2, (uint8_t)TANK_SPEED_BLACK},
};

Tank::Tank(const struct serialized_tank *ser_tank, uint8_t id):
    PhysicsBody(TANK_SIZE, TANK_SIZE),
    type(ser_tank->type),
    id(id),
    // add 1 because the level system uses coordinates from the first non-border block
    start_x(ser_tank->start_x + 1),
    start_y(ser_tank->start_y + 1) {

    tile_collisions = true;
    respect_holes = true;

    position_x = TILE_TO_X_COORD(start_x);
    position_y = TILE_TO_Y_COORD(start_y);

    if(id == 0) {
        game.player = this;
    }

    game.num_tanks++;
}

Tank::~Tank() {
    game.num_tanks--;
    if(this == game.player) {
        game.player = nullptr;
    }
}

void Tank::kill() {
    if(this != game.player) {
        game.total_kills++;
        game.kills[type]++;
        game.alive_tanks[id] = false;
    }

    active = false;
}

void Tank::process() {
    profiler_add(tanks);

    profiler_add(ai);
    ai_process_move(this);
    ai_process_fire(this);
    profiler_end(ai);

    if(shot_cooldown) {
        shot_cooldown--;
    }
    if(mine_cooldown) {
        mine_cooldown--;
    }

    profiler_end(tanks);
}

void Tank::render(uint8_t layer) {
    if(layer == 0) {
        tread_distance += abs(velocity_x) + abs(velocity_y);
        if(draw_treads) {
            gfx_TransparentSprite(tread_sprite, tread_pos.xmin, tread_pos.ymin);
            redraw_tiles(&tread_pos, 0);
            draw_treads = false;
        }
        if(tread_distance > TREAD_DISTANCE) {
            uint8_t base_index = (((uint8_t) -((tread_rot >> (INT_BITS - 8)) - 64)) >> 3) & 0xF;
            get_sprite_footprint(&tread_pos, this, tread_sprites, tread_x_offsets, tread_y_offsets, base_index);
            tread_distance -= TREAD_DISTANCE;
            tread_sprite = tread_sprites[base_index];
            gfx_TransparentSprite(tread_sprite, tread_pos.xmin, tread_pos.ymin);
            redraw_tiles(&tread_pos, 0);
            draw_treads = true;
        }
    }
    if(layer != 2) return;
    profiler_add(render_tanks);

    uint8_t base_index = (((uint8_t) -((tread_rot >> (INT_BITS - 8)) - 64)) >> 3) & 0xF;
    uint8_t turret_index = ((uint8_t) -((barrel_rot >> (INT_BITS - 8)) - 68)) >> 4;

    gfx_region_t base_region, turret_region, combined_region;

    gfx_sprite_t *base_sprite = base_cache.get(type, base_index);
    gfx_sprite_t *turret_sprite = turret_cache.get(type, turret_index);

    if(type == PLAYER) {
        get_sprite_footprint(&base_region, this, tank_bases[type], pl_base_x_offsets, pl_base_y_offsets, base_index);
        get_sprite_footprint(&turret_region, this, tank_turrets[type], pl_turret_x_offsets, pl_turret_y_offsets, turret_index);
    } else {
        get_sprite_footprint(&base_region, this, tank_bases[type], en_base_x_offsets, en_base_y_offsets, base_index);
        get_sprite_footprint(&turret_region, this, tank_turrets[type], en_turret_x_offsets, en_turret_y_offsets, turret_index);
    }

    combined_region.xmin = min(base_region.xmin, turret_region.xmin);
    combined_region.xmax = max(base_region.xmax, turret_region.xmax);
    combined_region.ymin = min(base_region.ymin, turret_region.ymin);
    combined_region.ymax = max(base_region.ymax, turret_region.ymax);

    if(pdraw_RectRegion(&combined_region)) {
        gfx_TransparentSprite(base_sprite, base_region.xmin, base_region.ymin);
        gfx_TransparentSprite(turret_sprite, turret_region.xmin, turret_region.ymin);
        redraw_tiles(&combined_region, 0);
    }

    profiler_end(render_tanks);
}

void Tank::fire_shell() {
    if(!can_shoot()) return;
    Shell *shell = new (std::nothrow) Shell(this);
    if(!shell) {
        dbg_printf("Failed to allocate fired shell\n");
        return;
    }

    num_shells++;
    shot_cooldown = SHOT_COOLDOWN;
}

bool Tank::can_shoot() const {
    return !shot_cooldown && num_shells < max_shells();
}

void Tank::lay_mine() {
    if(!can_lay_mine()) return;
    Mine *mine = new (std::nothrow) Mine(this);
    if(!mine) {
        dbg_printf("Failed to allocate mine\n");
        return;
    }

    num_mines++;
    mine_cooldown = MINE_COOLDOWN;
}

bool Tank::can_lay_mine() const {
    return !mine_cooldown && num_mines < max_mines();
}

void Tank::set_velocity(int velocity) {
    if(velocity == 0) {
        velocity_x = 0;
        velocity_y = 0;
    } else {
        velocity_x = velocity * fast_cos(tread_rot) / TRIG_SCALE;
        velocity_y = velocity * fast_sin(tread_rot) / TRIG_SCALE;
    }
}

void Tank::handle_collision(PhysicsBody *other) {
    other->collide(this);
}

void Tank::handle_explosion() {
    kill();
}

void Tank::collide(Tank *tank) {
    //Figure out if the four corners are colliding
    bool top_right = tank->is_point_inside(position_x + width, position_y);
    bool bottom_right = tank->is_point_inside(position_x + width, position_y + height);
    bool top_left = tank->is_point_inside(position_x, position_y);
    bool bottom_left = tank->is_point_inside(position_x, position_y + height);

    uint dis_up = -1;
    uint dis_down = -1;
    uint dis_left = -1;
    uint dis_right = -1;

    if(!(top_right || bottom_right || top_left || bottom_left)) return;

    if((top_right || bottom_right)) {
        dis_right = position_x + width - tank->position_x;
    }
    if((top_left || bottom_left)) {
        dis_left = tank->position_x + tank->width - position_x;
    }
    if((top_left || top_right)) {
        dis_up = tank->position_y + tank->height - position_y;
    }
    if((bottom_left || bottom_right)) {
        dis_down = position_y + height - tank->position_y;
    }

    //pick the direction with the smallest distance
    if(dis_up < dis_left && dis_up < dis_right) {
        velocity_y += dis_up / 2;
        tank->velocity_y -= dis_up / 2;
    }
    if(dis_left < dis_up && dis_left < dis_down) {
        velocity_x += dis_left / 2;
        tank->velocity_x -= dis_left / 2;
    }
    if(dis_down < dis_left && dis_down < dis_right) {
        velocity_y -= dis_down / 2;
        tank->velocity_y += dis_down / 2;
    }
    if(dis_right < dis_up && dis_right < dis_down) {
        velocity_x -= dis_right / 2;
        tank->velocity_x += dis_right / 2;
    }
}

void Tank::collide(Shell *shell) {
    shell->collide(this);
}

void Tank::collide(MineDetector *detector) {
    detector->collide(this);
}
