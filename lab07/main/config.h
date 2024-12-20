#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIG_GAME_TIMER_PERIOD 40.0E-3f

// Board
#define CONFIG_BOARD_R 6 // Rows
#define CONFIG_BOARD_C 7 // Columns
#define CONFIG_BOARD_N 4 // Number of contiguous marks

#define CONFIG_BOARD_SPACES (CONFIG_BOARD_R*CONFIG_BOARD_C)

// Colors
#define CONFIG_BACK_CLR rgb565(0, 79, 163)
#define CONFIG_GRID_CLR WHITE
#define CONFIG_MARK_CLR YELLOW
#define CONFIG_HIGH_CLR GREEN
#define CONFIG_MESS_CLR GREEN

#endif // CONFIG_H_
