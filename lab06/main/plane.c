#include "plane.h"
#include "lcd.h"
#include "config.h"
#include <stdio.h>
#include <math.h>

#define PLANE_Y_POS 20

// plane states
typedef enum {
    moving,
    idle,
} plane_state_t;

// plane
plane_t plane;
uint32_t launch_loc;
uint8_t shots;

/******************** Plane Init Function ********************/

// Initialize the plane state machine. Pass a pointer to the missile
// that will be (re)launched by the plane. It will only have one missile.
void plane_init(missile_t *plane_missile) {
    plane.currentState = idle;
    plane.timeout = CONFIG_PLANE_IDLE_TIME_TICKS;
    plane.missile = plane_missile;
    launch_loc = rand() % HW_LCD_W;
    shots = 1;
}

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void plane_explode(void) {
    plane.currentState = idle;
    plane.timeout = CONFIG_PLANE_IDLE_TIME_TICKS;
    launch_loc = rand() % HW_LCD_W;
    shots = 1;
}

// State machine tick function.
void plane_tick(void) {

    // state transitions
    switch(plane.currentState) {
        case idle:
            if (plane.timeout < 0) {
                plane.currentState = moving;
                plane.x_position = HW_LCD_W;
            }
            break;
        case moving:
            if (plane.x_position < -CONFIG_PLANE_WIDTH) {
                plane.currentState = idle;
                plane.timeout = CONFIG_PLANE_IDLE_TIME_TICKS;
                launch_loc = rand() % HW_LCD_W;
                shots = 1;
            }
            break;
    }

    // state actions
    switch(plane.currentState) {
        case idle:
            plane.timeout -= 1;
            break;
        case moving:
            plane.x_position -= CONFIG_PLANE_DISTANCE_PER_TICK;
            lcd_fillTriangle(plane.x_position, PLANE_Y_POS,
                (plane.x_position + CONFIG_PLANE_WIDTH), (PLANE_Y_POS - CONFIG_PLANE_HEIGHT/2),
                (plane.x_position + CONFIG_PLANE_WIDTH), (PLANE_Y_POS + CONFIG_PLANE_HEIGHT/2),
                CONFIG_COLOR_PLANE);
            
            if (plane.x_position < launch_loc && missile_is_idle(plane.missile) && shots > 0) {
                missile_init_plane(plane.missile, plane.x_position, PLANE_Y_POS);
                shots--;
            }
            break;
    }
}
/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void plane_get_pos(coord_t *x, coord_t *y) {
    *x = plane.x_position;
    *y = PLANE_Y_POS;
}

// Return whether the plane is flying.
bool plane_is_flying(void) {
    return (plane.currentState == moving);
}