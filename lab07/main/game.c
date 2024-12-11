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
    new_game_st,
    wait_mark_st,
    mark_st,
    wait_restart_st
} GameState;
static GameState state;
static bool x_turn;
int8_t r, c;

// Initialize the game logic.
void game_init(void) {
    state = init_st;
}

// Update the game logic.
void game_tick(void) {
    // state transitions
    switch(state) {
        case init_st:
            state = new_game_st;
            break;
        case new_game_st:
            state = wait_mark_st;
            break;
        case wait_mark_st:
        uint8_t buffer[1];
            // Wait for a mark (Button A press) from a player
            if (!pin_get_level(HW_BTN_A)) {
                // Get the navigator location
                nav_get_loc(&r, &c);
                // Check if the location is valid
                if (board_get(r, c) == no_m) {
                    // encode r and c into a single byte and send it to connected uart device
                    uint8_t rc = (r << NIBBLE) | c;
                    com_write(&rc, 1);
                    // If so, set mark on board and transition to mark_st
                    state = mark_st;
                    break;
                }
            }
            
            if (com_read(buffer, 1) > 0) {
                // decode r and c from the received byte
                r = ((uint8_t)buffer[0] & LEFT_NIBBLE) >> NIBBLE;
                c = (uint8_t)buffer[0] & RIGHT_NIBBLE;
                // Check if the location is valid
                if (board_get(r, c) == no_m) {
                    // If so, set mark on board and transition to mark_st
                    state = mark_st;
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
                if (x_turn) {
                    graphics_drawMessage("Red's turn", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                } else {
                    graphics_drawMessage("Yellow's turn", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
                }
            }
            break;
        case wait_restart_st:
            // Wait for a restart (Start Button press)
            if (!pin_get_level(HW_BTN_START)) {
                // Transition to new_game_st
                state = new_game_st;
            }
            break;
    }

    // state actions
    switch(state) {
        case init_st:
            break;
        case new_game_st:
            uint8_t temp[1];
            while (com_read(temp, 1) > 0) {
                // Clear the input buffer
            }

            // Draw game background and grid on the display
            lcd_fillScreen(CONFIG_BACK_CLR);
            graphics_drawGrid(CONFIG_GRID_CLR);
            // Clear the board
            board_clear();
            // X always goes first
            x_turn = true;
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
            // Draw an X or O on the display
            if (x_turn) {
                board_set(r, c, R_m);
                graphics_drawX(r, c, CONFIG_MARK_CLR);
            } else {
                board_set(r, c, Y_m);
                graphics_drawO(r, c, CONFIG_MARK_CLR);
            }
            // Next player's turn
            x_turn = !x_turn;
            break;
        case wait_restart_st:
            break;
    }

}
