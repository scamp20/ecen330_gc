#include "game.h"
#include "nav.h"
#include "board.h"
#include "pin.h"
#include "hw.h"
#include "graphics.h"
#include "lcd.h"
#include "config.h"
#include "com.h"

#define NIBBLE 4
#define LEFT_NIBBLE 0xF0
#define RIGHT_NIBBLE 0x0F

typedef enum {
    init_st,
    choose_player_st,
    new_game_st,
    wait_mark_st,
    mark_st,
    wait_restart_st
} GameState;
static GameState state;
static bool red_turn;
int8_t r, c;
bool player_red;
bool game_started = false;

// Initialize the game logic.
void game_init(void) {
    state = init_st;
}

// Update the game logic.
void game_tick(void) {
    // state transitions
    switch(state) {
        case init_st:
            game_started = false;
            lcd_fillScreen(CONFIG_BACK_CLR);
            graphics_draw_player_selection_display();
            state = choose_player_st;
            break;
        case choose_player_st:
            uint8_t sel_buffer[1];
            // Wait for a mark (Button A press) from a player
            if (!pin_get_level(HW_BTN_A)) {
                player_red = true;
                com_write(&player_red, 1);
                state = new_game_st;
                break;                                                                                    
            }
            
            if (com_read(sel_buffer, 1) > 0) {
                player_red = false;
                state = new_game_st;
            }
            break;
        case new_game_st:
            state = wait_mark_st;
            break;
        case wait_mark_st:
            uint8_t buffer[1];
            // Wait for a mark (Button A press) from a player
            if ((player_red && red_turn) || (!player_red && !red_turn)) {
                if (!pin_get_level(HW_BTN_A)) {
                    // Get the navigator location
                    nav_get_loc(&r, &c);
                    // Check if the column is valid
                    if (column_valid(c)) {
                        // encode r and c into a single byte and send it to connected uart device
                        uint8_t rc = (r << NIBBLE) | c;
                        com_write(&rc, 1);
                        // If so, set mark on board and transition to mark_st
                        state = mark_st;
                        break;                                                                                    
                    }
                }
            } else {
                if (com_read(buffer, 1) > 0) {
                    // decode r and c from the received byte
                    // r = ((uint8_t)buffer[0] & LEFT_NIBBLE) >> NIBBLE;
                    c = (uint8_t)buffer[0] & RIGHT_NIBBLE;
                    // Check if the location is valid
                    if (column_valid(c)) {
                        // If so, set mark on board and transition to mark_st
                        state = mark_st;
                    }

                }
            }
            break;
        case mark_st:
            // Check the board for a win or draw
            bool tie = false;
            mark_t winner = no_m;
            if (board_winner(R_m)) {
                winner = R_m;
            } else if (board_winner(Y_m)) {
                winner = Y_m;
            } else if (board_mark_count() == CONFIG_BOARD_SPACES) {
                tie = true;
            }

            // Transition to wait_mark_st or wait_restart_st
            if (winner != no_m || tie == true) {
                state = wait_restart_st;
            } else {
                state = wait_mark_st;
            }

            // Display status message (next player, winner, or draw)
            if (winner != no_m || tie == true) {
                if (winner == R_m) {
                    graphics_drawMessage("Red wins!", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                } else if (winner == Y_m) {
                    graphics_drawMessage("Yellow wins!", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                } else {
                    graphics_drawMessage("It's a tie!", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                }
            } else {
                if (red_turn) {
                    graphics_drawMessage("Red's turn", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                } else {
                    graphics_drawMessage("Yellow's turn", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                }
            }
            break;
        case wait_restart_st:
            // Wait for a restart (Start Button press)
            if (!pin_get_level(HW_BTN_START)) {
                // notify other player to restart the game
                uint8_t restart = 0xFF;
                com_write(&restart, 1);
                // Transition to new_game_st
                state = init_st;
                break;
            }

            uint8_t restart_buffer[1];
            if (com_read(restart_buffer, 1) > 0) {
                // Transition to new_game_st
                state = init_st;
            }
            break;
    }

    // state actions
    switch(state) {
        case init_st:
            break;
        case choose_player_st:
            uint8_t sel_temp[1];
            while (com_read(sel_temp, 1) > 0) {
                // Clear the input buffer
            }

            break;
        case new_game_st:
            game_started = true;

            uint8_t temp[1];
            while (com_read(temp, 1) > 0) {
                // Clear the input buffer
            }

            // Draw game background and grid on the display
            lcd_fillScreen(CONFIG_BACK_CLR);
            graphics_drawGrid(CONFIG_GRID_CLR);
            // Clear the board
            board_clear();
            // R always goes first
            red_turn = true;
            // Set the navigator location to the center
            nav_set_loc(1,1);
            // Transition to wait_mark_st
            state = wait_mark_st;
            // Display next player message
            graphics_drawMessage("Red's turn", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
            break;
        case wait_mark_st:
            break;
        case mark_st:
            // Draw a red or yellow circle on the display
            if (red_turn) {
                r = board_drop(c, R_m);
                graphics_draw_circle(r, c, RED);
            } else {
                r = board_drop(c, Y_m);
                graphics_draw_circle(r, c, YELLOW);
            }
            // Next player's turn
            red_turn = !red_turn;
            break;
        case wait_restart_st:
            break;
    }

}

bool started(void) {
    return game_started;
}
