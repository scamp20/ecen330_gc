#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define delayMS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

#define CAR_W 60
#define CAR_H 32

#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24
#define ROOF_X0 1
#define ROOF_Y0 0
#define ROOF_X1 39
#define ROOF_Y1 11
#define HOOD_X0 40
#define HOOD_Y0 9
#define HOOD_X1 40
#define HOOD_Y1 11
#define HOOD_X2 59
#define HOOD_Y2 11
#define WINDOW1_X0 3
#define WINDOW1_Y0 1
#define WINDOW1_X1 18
#define WINDOW1_Y1 8
#define WINDOW2_X0 21
#define WINDOW2_Y0 1
#define WINDOW2_X1 37
#define WINDOW2_Y1 8
#define WINDOW_RADIUS 2
#define WHEEL1_X 11
#define WHEEL2_X 48
#define WHEELS_Y 24
#define WHEEL_RADIUS 4
#define TIRE_RADIUS 7


/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{
	// roof
	lcd_fillRect2(x+ROOF_X0, y+ROOF_Y0, x+ROOF_X1, y+ROOF_Y1, CAR_CLR);

	// hood
	lcd_fillTriangle(x+HOOD_X0, y+HOOD_Y0, x+HOOD_X1, y+HOOD_Y1, x+HOOD_X2, y+HOOD_Y2, CAR_CLR);

	// body
	lcd_fillRect2(x+BODY_X0, y+BODY_Y0, x+BODY_X1, y+BODY_Y1, CAR_CLR);

	// windows
	lcd_fillRoundRect2(x+WINDOW1_X0, y+WINDOW1_Y0, x+WINDOW1_X1, y+WINDOW1_Y1, WINDOW_RADIUS, WINDOW_CLR);
	lcd_fillRoundRect2(x+WINDOW2_X0, y+WINDOW2_Y0, x+WINDOW2_X1, y+WINDOW2_Y1, WINDOW_RADIUS, WINDOW_CLR);

	// tires
	lcd_fillCircle(x+WHEEL1_X, y+WHEELS_Y, TIRE_RADIUS, TIRE_CLR);
	lcd_fillCircle(x+WHEEL2_X, y+WHEELS_Y, TIRE_RADIUS, TIRE_CLR);

	// wheel
	lcd_fillCircle(x+WHEEL1_X, y+WHEELS_Y, WHEEL_RADIUS, HUB_CLR);
	lcd_fillCircle(x+WHEEL2_X, y+WHEELS_Y, WHEEL_RADIUS, HUB_CLR);
}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


void app_main(void) {
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_setFontSize(FONT_SIZE);

	// Exercise 1 - Draw car in one location.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 1", TITLE_CLR);
	drawCar(OBJ_X, OBJ_Y);
	delayMS(WAIT);


	// Exercise 2 - Draw moving car (Method 1), one pass across display.
	// loop
	for (coord_t x = -CAR_W; x <= LCD_W + 1; x += OBJ_MOVE) {
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 2", TITLE_CLR);
		drawCar(x, OBJ_Y);
		char str[10];
		sprintf(str, "%3ld", x);
		lcd_drawString(LCD_W-STATUS_W, LCD_H-FONT_H, str, STATUS_CLR);
	}


	// Exercise 3 - Draw moving car (Method 2), one pass across display.
	// before loop
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 3", TITLE_CLR);

	// loop
	for (coord_t x = -CAR_W; x <= LCD_W + 1; x += OBJ_MOVE) {
		lcd_fillRect2(x - OBJ_MOVE, OBJ_Y, x - OBJ_MOVE + CAR_W, OBJ_Y + CAR_H, BACKGROUND_CLR);
		drawCar(x, OBJ_Y);
		char str[10];
		sprintf(str, "%3ld", x);
		lcd_fillRect2(LCD_W-STATUS_W, LCD_H-FONT_H, LCD_W, LCD_H, BACKGROUND_CLR);
		lcd_drawString(LCD_W-STATUS_W, LCD_H-FONT_H, str, STATUS_CLR);
		delayMS(20);
	}


	// Exercise 4 - Draw moving car (Method 3), one pass across display.
	// before loop
	lcd_frameEnable();

	// loop
	for (coord_t x = -CAR_W; x <= LCD_W + 1; x += OBJ_MOVE) {
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 4", TITLE_CLR);
		drawCar(x, OBJ_Y);
		char str[10];
		sprintf(str, "%3ld", x);
		lcd_drawString(LCD_W-STATUS_W, LCD_H-FONT_H, str, STATUS_CLR);
		lcd_writeFrame();
	}

	// after loop
	lcd_frameDisable();


	// Exercise 5 - Draw an animated Pac-Man moving across the display.
	// before loop
	lcd_frameEnable();

	// loop
	while(1) {
		uint16_t i = 0;
		const uint8_t pidx[] = {0, 1, 2, 1};

		for (coord_t x = -PAC_W; x <= LCD_W + 1; x += OBJ_MOVE) {
			lcd_fillScreen(BACKGROUND_CLR);
			lcd_drawString(0, 0, "Exercise 5", TITLE_CLR);
			lcd_drawBitmap(x, OBJ_Y, pac[pidx[i++ % sizeof(pidx)]], PAC_W, PAC_H, YELLOW);
			char str[10];
			sprintf(str, "%3ld", x);
			lcd_drawString(LCD_W-STATUS_W, LCD_H-FONT_H, str, STATUS_CLR);
			lcd_writeFrame();
		}
	}

	// after loop
	lcd_frameDisable();
}
