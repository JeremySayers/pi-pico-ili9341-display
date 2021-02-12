#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_MISO 14
#define PIN_CS   13
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_DC 15
#define PIN_RST 14
#define PIN_LED 25

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_TOTAL_PIXELS SCREEN_WIDTH * SCREEN_HEIGHT
#define BUFFER_SIZE SCREEN_TOTAL_PIXELS * 2

// display buffer of our screen size.
uint8_t buffer[BUFFER_SIZE];

void send(uint8_t command, uint8_t *data, size_t size)
{
    // set chip select to low so that the display starts listening.
    gpio_put(PIN_CS, 0);

    // set data/command to command mode (low).
    gpio_put(PIN_DC, 0);

    // send the command to the display.
    spi_write_blocking(SPI_PORT, &command, 1);

    // put the display back into data mode (high).
    gpio_put(PIN_DC, 1);

    // write the entire pixel buffer to the display (yolo).
    spi_write_blocking(SPI_PORT, data, size);

    // we're done transfering data to the display, tell it to stop listening.
    gpio_put(PIN_CS, 0);
}

void send_short(uint8_t command, uint16_t *data, size_t size)
{
    // set chip select to low so that the display starts listening.
    gpio_put(PIN_CS, 0);

    // set data/command to command mode (low).
    gpio_put(PIN_DC, 0);

    // send the command to the display.
    spi_write_blocking(SPI_PORT, &command, 1);

    // put the display back into data mode (high).
    gpio_put(PIN_DC, 1);

    // write the entire pixel buffer to the display (yolo).
    spi_write16_blocking(SPI_PORT, data, size);

    // we're done transfering data to the display, tell it to stop listening.
    gpio_put(PIN_CS, 0);
}

void init_display() 
{
    uint8_t displayInitCommands[] = 
    {
        0x0F, 3, 0x03, 0x80, 0x02,
        0xCF, 3, 0x00, 0xC1, 0x30,
        0xED, 4, 0x64, 0x03, 0x12, 0x81,
        0xE8, 3, 0x85, 0x00, 0x78,
        0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
        0xF7, 1, 0x20,
        0xEA, 2, 0x00, 0x00,
        0xC0, 1, 0x23,
        0xC1, 1, 0x10,
        0xC5, 2, 0x3e, 0x28,
        0XC7, 1, 0x86,
        0x36, 1, 0x48,
        0x37, 1, 0x00,
        0x3A, 1, 0x55,
        0xB1, 2, 0x00, 0x18,
        0xB6, 3, 0x08, 0x82, 0x27,
        0xF2, 1, 0x00,
        0xE0, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
        0xE1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
        0x11, 1
    };

    gpio_put(PIN_RST, 0);
    sleep_ms(500);
    gpio_put(PIN_RST, 1);
    sleep_ms(500);

    uint8_t cmd, x, numArgs;
    uint8_t *addr = displayInitCommands;

    // 20 is the number of commands in init, if you add or remove any this breaks...
    for (uint16_t i = 0; i < 20; i++)
    {
        cmd = *addr++;
        x = *addr++;
        numArgs = x & 0x7F;

        send(cmd, addr, numArgs);
        addr += numArgs;
    }
}

void init_SPI()
{
    // set up the SPI interface.
    spi_init(SPI_PORT, 4000000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RST);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_RST, 0);
}

void write_buffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t data[] = {x0, x1};

    send_short(0x2A, data, 2);

    data[0] = y0;
    data[1] = y1;

    send_short(0x2B, data, 2);
    send(0x2C, buffer, BUFFER_SIZE);
}

int main()
{
    stdio_init_all();
        
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 1);
    sleep_ms(500);
    gpio_put(PIN_LED, 0);
    sleep_ms(500);
    gpio_put(PIN_LED, 1);
    sleep_ms(500);
    gpio_put(PIN_LED, 0);

    init_SPI();
    init_display();

    while(1)
    {
        for (uint i = 0; i < BUFFER_SIZE; i++)
        {
            buffer[i] = 0xFF;
        }
        write_buffer(0, 0, 320, 240);

        sleep_ms(1000);

        for (uint i = 0; i < BUFFER_SIZE; i++)
        {
            buffer[i] = 0x00;
        }
        write_buffer(0, 0, 320, 240);

        sleep_ms(1000);
    }

    return 0;
}
