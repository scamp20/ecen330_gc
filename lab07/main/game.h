#ifndef GAME_H_
#define GAME_H_
#include <stdbool.h>

// Initialize the game logic.
void game_init(void);

// Update the game logic.
void game_tick(void);

// check if the game has started
bool started(void);

#endif // GAME_H_
