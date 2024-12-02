#include <math.h>
#include <stdlib.h>
#include "missile.h"
#include "config.h"
#include "hw.h"
#include "lcd.h"

#include <stdio.h>


// The same missile structure is used for all missiles in the game.
// All state variables for a missile are contained in the missile structure.
// There are no global variables maintained in the missile.c file.
// Each missile function requires a pointer argument to a missile struct.

typedef enum {
	initializing,
    moving,
    exploding_growing,
    exploding_shrinking,
    impacted,
    idle,
} state_t;

/******************** Helper Functions ********************/
float pythagorean(coord_t x_dest, coord_t x_origin, coord_t y_dest, coord_t y_origin) {
    return sqrt(pow(x_dest-x_origin, 2) + pow(y_dest-y_origin, 2));
}

/******************** Missile Init Functions ********************/

// Different _init_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. If initialized to the idle
// state, a missile doesn't appear nor does it move.
void missile_init_idle(missile_t *missile) {
    missile->currentState = idle;
}

// Initialize the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the
// closest "firing location" to the destination (there are three firing
// locations evenly spaced along the bottom of the screen).
void missile_init_player(missile_t *missile, coord_t x_dest, coord_t y_dest) {
    missile->type = MISSILE_TYPE_PLAYER;
    missile->currentState = initializing;
    if (x_dest < 3*HW_LCD_W/8) {
        missile->x_origin = HW_LCD_W/4;
    } else if (x_dest < 5*HW_LCD_W/8) {
        missile->x_origin = HW_LCD_W/2;
    } else {
        missile->x_origin = 3*HW_LCD_W/4;
    }
    missile->y_origin = HW_LCD_H;
    missile->x_dest = x_dest;
    missile->y_dest = y_dest;
    missile->total_length = pythagorean(x_dest, missile->x_origin, y_dest, missile->y_origin);
    missile->x_current = missile->x_origin;
    missile->y_current = missile->y_origin;
    missile->length = 0;
    missile->explode_me = false;
    missile->radius = 0;
}

// Initialize the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_init_enemy(missile_t *missile) {
    missile->type = MISSILE_TYPE_ENEMY;
    missile->currentState = initializing;
    missile->x_origin = rand() % HW_LCD_W; 
    missile->y_origin = rand() % (HW_LCD_H/8);
    missile->x_dest = rand() % HW_LCD_W;
    missile->y_dest = HW_LCD_H;
    missile->total_length = pythagorean(missile->x_dest, missile->x_origin, missile->y_dest, missile->y_origin);
    missile->x_current = missile->x_origin;
    missile->y_current = missile->y_origin;
    missile->length = 0;
    missile->explode_me = false;
    missile->radius = 0;
}

// Initialize the missile as a plane missile. This function takes the (x, y)
// location of the plane as an argument and uses it as the missile origin.
// The destination is randomly chosen along the bottom of the screen.
void missile_init_plane(missile_t *missile, coord_t x_orig, coord_t y_orig) {
    missile->type = MISSILE_TYPE_PLANE;
    missile->currentState = initializing;
    missile->x_origin = x_orig; 
    missile->y_origin = y_orig;
    missile->x_dest = rand() % HW_LCD_W;
    missile->y_dest = HW_LCD_H;
    missile->total_length = pythagorean(missile->x_dest, missile->x_origin, missile->y_dest, missile->y_origin);
    missile->x_current = missile->x_origin;
    missile->y_current = missile->y_origin;
    missile->length = 0;
    missile->explode_me = false;
    missile->radius = 0;
}

/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy or a plane missile is located within an explosion zone.
void missile_explode(missile_t *missile) {
    missile->explode_me = true;
}

// Tick the state machine for a single missile.
void missile_tick(missile_t *missile) {

    // state transitions
    switch (missile->currentState) {
        case initializing:
            missile->currentState = moving;
            break;
        case moving:
            if (missile->explode_me) {
                missile->currentState = exploding_growing;
            } else if ((missile->type != MISSILE_TYPE_PLAYER) && (missile->length >= missile->total_length)) {
                missile->currentState = impacted;
            }
            break;
        case exploding_growing:
            if (missile->radius >= CONFIG_EXPLOSION_MAX_RADIUS) {
                missile->currentState = exploding_shrinking;
            }
            break;
        case exploding_shrinking:
            if (missile->radius <= 0) {
                missile->currentState = idle;
            }
            break;
        case impacted:
            missile->currentState = idle;
            break;
        case idle:
            break;
    }

    // state actions
    color_t color;
    switch (missile->currentState) {
        case initializing:
            break;
        case moving:
            float missile_distance_per_tick;

            if (missile->type == MISSILE_TYPE_PLAYER) {
                missile_distance_per_tick = CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK;
                color = CONFIG_COLOR_PLAYER_MISSILE;
            } else if (missile->type == MISSILE_TYPE_ENEMY) {
                missile_distance_per_tick = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
                color = CONFIG_COLOR_ENEMY_MISSILE;
            } else {
                missile_distance_per_tick = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
                color = CONFIG_COLOR_PLANE_MISSILE;
            }

            missile->length += missile_distance_per_tick;
            float fraction = missile->length / missile->total_length;
            missile->x_current = missile->x_origin + fraction * (missile->x_dest - missile->x_origin);
            missile->y_current = missile->y_origin + fraction * (missile->y_dest - missile->y_origin);
            lcd_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, color);

            // player missile explosion upon arrival
            if (missile->type == MISSILE_TYPE_PLAYER && missile->length >= missile->total_length) {
                missile_explode(missile);
            }
            break;
        case exploding_growing:
            if (missile->type == MISSILE_TYPE_PLAYER) {
                color = CONFIG_COLOR_PLAYER_MISSILE;
            } else if (missile->type == MISSILE_TYPE_ENEMY) {
                color = CONFIG_COLOR_ENEMY_MISSILE;
            } else {
                color = CONFIG_COLOR_PLANE_MISSILE;
            }
            missile->radius += CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, color);
            break;
        case exploding_shrinking:
            if (missile->type == MISSILE_TYPE_PLAYER) {
                color = CONFIG_COLOR_PLAYER_MISSILE;
            } else if (missile->type == MISSILE_TYPE_ENEMY) {
                color = CONFIG_COLOR_ENEMY_MISSILE;
            } else {
                color = CONFIG_COLOR_PLANE_MISSILE;
            }
            missile->radius -= CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, color);
            break;
        case impacted:
            break;
        case idle:
            break;
    }
}

/******************** Missile Status Functions ********************/

// Return the current missile position through the pointers *x,*y.
void missile_get_pos(missile_t *missile, coord_t *x, coord_t *y) {
    *x = missile->x_current;
    *y = missile->y_current;
}

// Return the missile type.
missile_type_t missile_get_type(missile_t *missile) {
    return missile->type;
}

// Return whether the given missile is moving.
bool missile_is_moving(missile_t *missile) {
    return missile->currentState == moving;
}

// Return whether the given missile is exploding. If this missile
// is exploding, it can explode another intersecting missile.
bool missile_is_exploding(missile_t *missile) {
    return (missile->currentState == exploding_growing || missile->currentState == exploding_shrinking);
}

// Return whether the given missile is idle.
bool missile_is_idle(missile_t *missile) {
    return missile->currentState == idle;
}

// Return whether the given missile is impacted.
bool missile_is_impacted(missile_t *missile) {
    return missile->currentState == impacted;
}

// Return whether an object (e.g., missile or plane) at the specified
// (x,y) position is colliding with the given missile. For a collision
// to occur, the missile needs to be exploding and the specified
// position needs to be within the explosion radius.
bool missile_is_colliding(missile_t *missile, coord_t x, coord_t y) {
    return pythagorean(x, missile->x_current, y, missile->y_current) <= missile->radius;
}