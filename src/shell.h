#ifndef TANKS_SHELL_H
#define TANKS_SHELL_H

#include <stdbool.h>
#include "collision.h"

#define SHELL_SIZE (TILE_SIZE * 3 / 14)

//Speed of shells in pixels per tick
//Standard:
//434 px X / 48 px/tile = 11.4210526316 tiles
//157 px Y / 32 px/tile = 4.90625 tiles
//d = 12.4302748271 tile
//123 frames=2.05 seconds
//6.06354869615 tiles/second
#define SHELL_SPEED_STANDARD (6.06354869615 * TILE_SIZE / TARGET_FPS)
//Fast
//238 px X / 48 px/tile = 4.95833333333 tiles
//37  px Y / 32 px/tile = 1.15625 tiles
//d = 5.09136361959 tiles
//30 frames = 1/2 second
//10.1827272392 tiles/second
#define SHELL_SPEED_MISSILE (10.1827272392 * TILE_SIZE / TARGET_FPS)

typedef struct {
    bool alive; //Whether this shell is processed
    physicsBody_t phys;
    uint8_t bounces; //Number of times the shell can bounce off a wall without exploding
    bool left_tank_hitbox; //Whether the shell has exited the tank hitbox yet. Used to stop shells from blowing up the tank that fired them.
    uint8_t direction;
} shell_t;

//Bounce a shell off a wall
//Returns whether or not the shell is still alive
bool shellRicochet(shell_t* shell, direction_t dir);

uint8_t inline angleToShellDirection(angle_t angle) {
    return ((uint8_t)-((angle >> 16) - 64)) >> 4;
}
void updateShellDirection(shell_t* shell);

#endif //TANKS_SHELL_H
