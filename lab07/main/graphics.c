#include "lcd.h"
#include "graphics.h"
#include "config.h"

#define MESS_FONT_SZ 1

#define MESS_X (LCD_CHAR_W*MESS_FONT_SZ)
#define MESS_Y (LCD_H-MESS_H)
#define MESS_W (LCD_W-LCD_CHAR_W*MESS_FONT_SZ*2)
#define MESS_H (LCD_CHAR_H*MESS_FONT_SZ)

#define VIEW_X (LCD_CHAR_W*MESS_FONT_SZ)
#define VIEW_Y 0
#define VIEW_W (LCD_W-LCD_CHAR_W*MESS_FONT_SZ*2)
#define VIEW_H (LCD_H-MESS_H)

#define GRID_W CONFIG_BOARD_C
#define GRID_H CONFIG_BOARD_R

// Dimensions in pixels
#define CELL_W (VIEW_W/GRID_W)
#define CELL_H (VIEW_H/GRID_H)

#if CELL_W < 35 || CELL_H < 35
#define HIGH_MARGIN 2
#define MARK_MARGIN 4
#else
#define HIGH_MARGIN 6
#define MARK_MARGIN 12
#endif

#if LCD_H < LCD_W
  #define MARK_SZ (CELL_H - 2*MARK_MARGIN)
#else
  #define MARK_SZ (CELL_W - 2*MARK_MARGIN)
#endif

void graphics_drawGrid(color_t color)
{
	// draw circles in between the grid lines
	for (int8_t i = 0; i < GRID_W; i++) {
		for (int8_t j = 0; j < GRID_H; j++) {
			coord_t x = VIEW_X + i * CELL_W + CELL_W/2;
			coord_t y = VIEW_Y + j * CELL_H + CELL_H/2;
			lcd_fillCircle(x, y, 15, color);
		}
	}
}

void graphics_drawMessage(const char *str, color_t color, color_t bg)
{
	lcd_fillRect(MESS_X, MESS_Y, MESS_W, MESS_H, bg);
	lcd_setFontSize(MESS_FONT_SZ);
	lcd_drawString(MESS_X, MESS_Y, str, color);
}

void graphics_draw_red(int8_t r, int8_t c, color_t color)
{
	coord_t xc = VIEW_X + c * CELL_W + CELL_W/2;
	coord_t yc = VIEW_Y + r * CELL_H + CELL_H/2;

	lcd_fillCircle(xc, yc, 15, RED);
}

void graphics_draw_yellow(int8_t r, int8_t c, color_t color)
{
	coord_t xc = VIEW_X + c * CELL_W + CELL_W/2;
	coord_t yc = VIEW_Y + r * CELL_H + CELL_H/2;

	lcd_fillCircle(xc, yc, 15, YELLOW);
}

void graphics_drawHighlight(int8_t c, color_t color)
{
	coord_t x = VIEW_X + c * CELL_W;
	coord_t y = VIEW_Y;

	lcd_drawRect(x+HIGH_MARGIN - 5, y,
		CELL_W-2*HIGH_MARGIN+10, LCD_H - (CELL_H/3), color);
}
