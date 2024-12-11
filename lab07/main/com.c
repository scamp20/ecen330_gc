#ifndef COM_H_
#define COM_H_

#include <stdint.h>
#include <string.h>
#include <driver/uart.h>
#include "hw.h"
#include "pin.h"
#include "com.h"

#define BAUD_RATE 115200

// This component is a wrapper around a lower-level communication
// method such as a serial port (UART), Bluetooth, or WiFi. The
// complexities of establishing a communication channel and sending
// bytes through the channel are abstracted away. The read and write
// functions are non-blocking so they can be used in a tick function.

// Initialize the communication channel.
// Return zero if successful, or non-zero otherwise.
int32_t com_init(void) {
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

    // Set UART pins(TX, RX, RTS, CTS)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, HW_EX8, HW_EX7, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Install UART driver using an event queue here (port, RX_buffer_size, TX_buffer_size, queue_size, uart_queue, interrupt flags)
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, (UART_HW_FIFO_LEN(PORT_NUM)*2), 0, 0, NULL, 0));

    pin_pullup(HW_EX7, true);
    return 0;
}

// Free resources used for communication.
// Return zero if successful, or non-zero otherwise.
int32_t com_deinit(void) {
    if (uart_is_driver_installed(UART_NUM_2)) {
        // Uninstall UART driver
        ESP_ERROR_CHECK(uart_driver_delete(UART_NUM_2));
    }
    return 0;
}

// Write data to the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to write
// Return number of bytes written, or negative number if error.
int32_t com_write(const void *buf, uint32_t size) {
    int32_t written = uart_tx_chars(UART_NUM_2, (const char*)buf, size);
    return written;
}

// Read data from the communication channel. Does not wait for data.
// *buf: pointer to data buffer
// size: size of data in bytes to read
// Return number of bytes read, or negative number if error.
int32_t com_read(void *buf, uint32_t size) {
    int32_t read = uart_read_bytes(UART_NUM_2, buf, size, 0);
    return read;
}

#endif // COM_H_
