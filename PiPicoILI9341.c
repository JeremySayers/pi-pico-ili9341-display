#include <stdio.h>
#include <stdlib.h>
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

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define SCREEN_TOTAL_PIXELS SCREEN_WIDTH * SCREEN_HEIGHT
#define BUFFER_SIZE SCREEN_TOTAL_PIXELS * 2

// display buffer of our screen size.
uint8_t buffer[BUFFER_SIZE];
uint8_t interlacePosition = 0;

uint actualBaudrate = 0;

struct Square
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int8_t xVelocity;
    int8_t yVelocity;
    uint8_t color;
};

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

static inline void dc_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_DC, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void dc_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_DC, 1);
    asm volatile("nop \n nop \n nop");
}

static void inline send_byte(uint8_t data)
{   
    cs_select();
    dc_deselect();
    spi_write_blocking(SPI_PORT, &data, 1);
    cs_deselect();
}

static void inline send_short(uint16_t data)
{
    cs_select();
    dc_deselect();

    uint8_t shortBuffer[2];

    shortBuffer[0] = (uint8_t) (data >> 8);
    shortBuffer[1] = (uint8_t) data;

    spi_write_blocking(SPI_PORT, shortBuffer, 2);

    cs_deselect();
}

static void inline send_command(uint8_t command)
{
    // set data/command to command mode (low).
    dc_select();

    cs_select();

    // send the command to the display.
    spi_write_blocking(SPI_PORT, &command, 1);

    // put the display back into data mode (high).
    dc_deselect();

    cs_deselect();
}

void init_display() 
{
    cs_select();

    gpio_put(PIN_RST, 0);
    sleep_ms(500);
    gpio_put(PIN_RST, 1);
    sleep_ms(500);

    // yes this is garbage, but I wanted to break it all down
    // when I was debugging what my initialization issues were.
    send_command(0xEF);
    send_byte(0x03);
    send_byte(0x80);
    send_byte(0x02);

    send_command(0xCF);
    send_byte(0x00);
    send_byte(0xC1);
    send_byte(0x30);

    send_command(0xED);
    send_byte(0x64);
    send_byte(0x03);
    send_byte(0x12);
    send_byte(0x81);
    
    send_command(0xE8);
    send_byte(0x85);
    send_byte(0x00);
    send_byte(0x78);

    send_command(0xCB);
    send_byte(0x39);
    send_byte(0x2C);
    send_byte(0x00);
    send_byte(0x34);
    send_byte(0x02);

    send_command(0xF7);
    send_byte(0x20);

    send_command(0xEA);
    send_byte(0x00);
    send_byte(0x00);

    send_command(0xC0);
    send_byte(0x23);
    
    send_command(0xC1);
    send_byte(0x10);

    send_command(0xC5);
    send_byte(0x3e);
    send_byte(0x28);

    send_command(0xC7);
    send_byte(0x86);

    send_command(0x36);
    send_byte(0x48);

    send_command(0x3A);
    send_byte(0x55);

    send_command(0xB1);
    send_byte(0x00);
    send_byte(0x1F); // 61 Hz
    //send_byte(10); // 119 Hz

    send_command(0xB6);
    send_byte(0x08);
    send_byte(0x82);
    send_byte(0x27);

    send_command(0xF2);
    send_byte(0x00);

    send_command(0x26);
    send_byte(0x01);

    send_command(0xE0);
    send_byte(0x0F);
    send_byte(0x31);
    send_byte(0x2B);
    send_byte(0x0C);
    send_byte(0x0E);
    send_byte(0x08);
    send_byte(0x4E);
    send_byte(0xF1);
    send_byte(0x37);
    send_byte(0x07);
    send_byte(0x10);
    send_byte(0x03);
    send_byte(0x0E);
    send_byte(0x09);
    send_byte(0x00);

    send_command(0xE1);
    send_byte(0x00);
    send_byte(0x0E);
    send_byte(0x14);
    send_byte(0x03);
    send_byte(0x11);
    send_byte(0x07);
    send_byte(0x31);
    send_byte(0xC1);
    send_byte(0x48);
    send_byte(0x08);
    send_byte(0x0F);
    send_byte(0x0C);
    send_byte(0x31);
    send_byte(0x36);
    send_byte(0x0F);

    send_command(0x11);

    sleep_ms(120);

    send_command(0x29);

    sleep_ms(120);

    send_command(0x13);
}

void init_SPI()
{
    // set up the SPI interface.
    spi_init(SPI_PORT, 62500000);
    actualBaudrate = spi_set_baudrate(SPI_PORT, 70000000);

    printf("Actual Baudrate: %i", actualBaudrate);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
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

void init_drawing()
{
    send_command(0x2A);
    send_short(0);
    send_short(239);

    send_command(0x2B);
    send_short(0);
    send_short(319);

    sleep_ms(10);

    send_command(0x2C);

    cs_select();
    dc_deselect();
}

inline void write_buffer()
{
    spi_write_blocking(SPI_PORT, buffer, BUFFER_SIZE);
}

void write_buffer_interlaced()
{
    send_command(0x2A);
    send_short(0);
    send_short(239);

    for (int i = interlacePosition; i < SCREEN_HEIGHT; i+=2)
    {
        send_command(0x2B);
        send_short(i);
        send_short(i+1);

        send_command(0x2C);

        cs_select();
        dc_deselect();

        spi_write_blocking(SPI_PORT, &buffer[i * SCREEN_WIDTH * 2], SCREEN_WIDTH * 2);
    }

    if (interlacePosition == 1) 
    {
        interlacePosition = 0;
    } 
    else 
    {
        interlacePosition = 1;
    }
}

inline void clear_buffer()
{
    for (uint i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 0x00;
    }
}

void inline draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color)
{
    for (int i = y * SCREEN_WIDTH * 2; i < (SCREEN_WIDTH * y * 2) + (h * SCREEN_WIDTH * 2); i+=SCREEN_WIDTH * 2)
    {
        for (int j = x * 2; j < (x * 2) + (w * 2); j+=2)
        {
            buffer[i+j] = color;
            buffer[i+j+1] = color;
        }
    }    
}

void update(struct Square player[], uint playerCount)
{
    for (int i = 0; i < playerCount; i++)
        {
            player[i].x += player[i].xVelocity;
            player[i].y += player[i].yVelocity;
            
            if (player[i].x <= 0)
            {
                player[i].x = 0;
                player[i].xVelocity = -player[i].xVelocity;
            }

            if (player[i].x >= SCREEN_WIDTH - player[i].w)
            {
                player[i].x = SCREEN_WIDTH - player[i].w;
                player[i].xVelocity = -player[i].xVelocity;
            }

            if (player[i].y <= 0)
            {
                player[i].y = 0;
                player[i].yVelocity = -player[i].yVelocity;
            }

            if (player[i].y >= SCREEN_HEIGHT - player[i].h)
            {
                player[i].y = SCREEN_HEIGHT - player[i].h;
                player[i].yVelocity = -player[i].yVelocity;
            }
            //printf("Player[%i]: (%i, %i) (%i, %i)\n", i, player[i].x, player[i].y, player[i].xVelocity, player[i].yVelocity);
        }
}

int main()
{
    stdio_init_all();

    printf("Starting up.\n");

    init_SPI();

    printf("SPI initialized.\n");

    init_display();

    printf("Display initialized.\n");

    init_drawing();

    printf("Drawing initialized.\n");

    uint playerCount = 50;

    struct Square player[playerCount];

    for (int i = 0; i < playerCount; i++)
    {
        player[i].x = rand() % 209;
        player[i].y = rand() % 289;
        player[i].w = 30;
        player[i].h = 30;
        player[i].xVelocity = rand() % 4 - 2;
        player[i].yVelocity = rand() % 4 - 2;

        if (player[i].xVelocity == 0 && player[i].yVelocity == 0)
        {
            player[i].xVelocity = 3;
            player[i].yVelocity = 3;
        }

        player[i].color = rand() % 255;
    }    

    while(1)
    {        
        update(player, playerCount);

        clear_buffer();

        for (int i = 0; i < playerCount; i++)
        {
            draw_rectangle(player[i].x, player[i].y, player[i].w, player[i].h, player[i].color);
        }

        uint32_t beforeWriteTime = time_us_32();
        write_buffer();
        //write_buffer_interlaced();
        uint32_t afterWriteTime = time_us_32();
        //printf("\n%ldus\n", afterWriteTime-beforeWriteTime);
        printf("Actual Buadrate: %i", actualBaudrate);
    }


    return 0;
}