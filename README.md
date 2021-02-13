# pi-pico-ili9341-display
A basic driver for interfacing the ili9341 display controller with the Pi Pico utilizing SPI.

Offsets for the drawing must be wrong since I'm getting a single empty line at the top (with the first byte of my buffer being written to the first pixel of the second line).

Seems to be able to write the full buffer pretty quickly, hoping that I can get 30FPS (should only need the SPI interface to operate at 36.864MHz which seems resonable).
