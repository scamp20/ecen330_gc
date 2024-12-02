
#include <stdio.h>
#include <stdlib.h> // rand

#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"
#include "missile.h"
#include "plane.h"
#include "gameControl.h"
#include "config.h"
#include "missileLaunch.h"

// sound support
#include "missileLaunch.h"

// M3: Define stats constants
#define STATUS_Y_POS 5
#define STATUS_X_POS 50
#define STATUS_X_POS2 150
#define VOLUME 20
#define STATUS_SIZE 20

// All missiles
missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];

// Alias into missiles array
missile_t *enemy_missiles = missiles+0;
missile_t *player_missiles = missiles+CONFIG_MAX_ENEMY_MISSILES;
missile_t *plane_missile = missiles+CONFIG_MAX_ENEMY_MISSILES+
									CONFIG_MAX_PLAYER_MISSILES;

// One shot button protection
static bool pressed = false;
coord_t x, y;
uint64_t btns;

// M3: Declare stats variables
uint32_t shot;
uint32_t impacted;

// Initialize the game control logic.
// This function initializes all missiles, planes, stats, etc.
void gameControl_init(void)
{
	// Initialize missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
		missile_init_enemy(enemy_missiles+i);
	for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++)
		missile_init_idle(player_missiles+i);
	missile_init_idle(plane_missile);

	// M3: Initialize plane
	plane_init(plane_missile);

	// M3: Initialize stats
	shot = 0;
	impacted = 0;

	// M3: Set sound volume
	sound_set_volume(VOLUME);
}

// Update the game control logic.
// This function calls the missile & plane tick functions, reinitializes
// idle enemy missiles, handles button presses, fires player missiles,
// detects collisions, and updates statistics.
void gameControl_tick(void)
{
	// Tick missiles in one batch
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
		missile_tick(missiles+i);

	// Reinitialize idle enemy missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
		if (missile_is_idle(enemy_missiles+i))
			missile_init_enemy(enemy_missiles+i);

	// M2: Check for button press. If so, launch a free player missile.
	btns = ~pin_get_in_reg() & HW_BTN_MASK;
	if (!pressed && btns) {
		pressed = true; // button pressed
		cursor_get_pos(&x, &y);
		// Check to see if a player missile is idle and launch it to the target (x,y) position.
		for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++) {
			if (missile_is_idle(player_missiles+i)) {
				missile_init_player(player_missiles+i, x, y);
				sound_start(missileLaunch, sizeof(missileLaunch), false);
				shot++;
				break;
			}
		}

	} else if (pressed && !btns) {
		pressed = false; // all released
	}

	// M2: Check for moving non-player missile collision with an explosion.
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) {

		// skip player missiles
		if (i == CONFIG_MAX_ENEMY_MISSILES) {
			i += CONFIG_MAX_PLAYER_MISSILES;
			if (i >= CONFIG_MAX_TOTAL_MISSILES) {  // Just safety in case max plane missiles == 0
				break;
			}
		}

		if (missile_is_moving(missiles+i)) {
			for (uint32_t j = 0; j < CONFIG_MAX_TOTAL_MISSILES; j++) {
				if (missile_is_exploding(missiles+j)) {
					missile_t* missile = missiles+i;
					if (missile_is_colliding(missiles+j, missile->x_current, missile->y_current)) {
						missile_explode(missile);
					}
				}
			}
		}
	}

	// M3: Count non-player impacted missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++) {
		if (missile_is_impacted(enemy_missiles+i)) {
			impacted++;
		}
	}

	if (missile_is_impacted(plane_missile)) {
		impacted++;
	}
		
	// M3: Tick plane & draw stats
	plane_tick();
	char status[STATUS_SIZE];
	sprintf(status, "Shot: %ld", shot);
	lcd_drawString(STATUS_X_POS, STATUS_Y_POS, status, CONFIG_COLOR_STATUS);
	sprintf(status, "Impacted: %ld", impacted);
	lcd_drawString(STATUS_X_POS2, STATUS_Y_POS, status, CONFIG_COLOR_STATUS);

	// M3: Check for flying plane collision with an explosion.
	if (plane_is_flying()) {
		plane_get_pos(&x, &y);
		for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) {
			if (missile_is_exploding(missiles+i)) {
				if (missile_is_colliding(missiles+i, x, y)) {
					plane_explode();
					break;
				}
			}
		}
	}
}
