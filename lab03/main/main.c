#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gptimer.h"
#include "hw.h" // defines I/O pins associated with buttons
#include "lcd.h"
#include "pin.h"
#include "watch.h"
#include "esp_timer.h"

#define ISR_COUNT_LIMIT 500
#define TIMER_PRESCALE 1000000
#define ALARM_COUNT 10000

static const char *TAG = "lab03";
static volatile uint32_t timer_ticks; // global variables initialize to 0 automatically
static bool running;

volatile int64_t isr_max; // Maximum ISR execution time (us)
volatile int32_t isr_cnt; // Count of ISR invocations

static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    int64_t start, finish;
    start = esp_timer_get_time();
    if (running) timer_ticks++;

    if (!pin_get_level(HW_BTN_A)) running = true;
    else if (!pin_get_level(HW_BTN_B)) running = false;
    else if (!pin_get_level(HW_BTN_START)) {
        running = false;
        timer_ticks = 0;
    }

    finish = esp_timer_get_time();
    if (finish-start > isr_max) isr_max = finish-start;
    isr_cnt++;
    return false;
}

// int64_t start, finish;
// start = esp_timer_get_time();
// finish = esp_timer_get_time();
// printf("Section time:%lld microseconds\n", finish-start);

// Main application
void app_main(void)
{
    int64_t start, finish;
    start = esp_timer_get_time();

    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_B);
    pin_reset(HW_BTN_START);
    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    pin_input(HW_BTN_START, true);

    finish = esp_timer_get_time();
    printf("Configure I/O pins:%lld microseconds\n", finish-start);

	ESP_LOGI(TAG, "Starting");

    start = esp_timer_get_time();
    // set up timer and timer configuration
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_PRESCALE, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // set callback function
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb, // register user callback
    };

    // register callback function for the timer
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // set up alarm configuration
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0, // counter will reload with 0 on alarm event
        .alarm_count = ALARM_COUNT, // period = 1s @resolution 1MHz
        .flags.auto_reload_on_alarm = true, // enable auto-reload
    };

    // attach alarm configuration to timer
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // enable and start the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    finish = esp_timer_get_time();
    printf("Configure stopwatch timer:%lld microseconds\n", finish-start);

    start = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update");
    finish = esp_timer_get_time();
    printf("ESP_LOGI(TAG, Stopwatch update); function:%lld microseconds\n", finish-start);

    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face

    for (;;) { // forever update loop
        watch_update(timer_ticks);
        if (isr_cnt >= ISR_COUNT_LIMIT) {
            printf("Stopwatch timer ISR:%lld microseconds\n", isr_max);
            isr_cnt = 0;
            isr_max = 0;
        }
    }
}