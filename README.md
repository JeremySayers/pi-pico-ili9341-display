# pi-pico-ili9341-display
A basic driver for interfacing the ili9341 display controller with the Pi Pico utilizing SPI.

Drawing is now fully working with two different modes! With the pico running at it's full SPI clock (62.5Mhz) I'm able to get about 40 FPS. Still investigating if I can overlock the pico AND the spi speed (when I overclock the pico it drops down to 22MHz SPI speed).

The other mode is interlaced, which sends the odd lines one frame, and the even the next, and that's doing about 75FPS with very minimal artifacts.
