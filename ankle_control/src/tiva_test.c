/// \file
/// \brief Display Led's RED, GREEN, BLUE, WHITE, each for one second
/// Designed for use with the EK-TM4C123GXL board
#include "nuhal/error.h"
#include "nuhal/tiva.h"
#include "nuhal/time"
#include "nuhal/led.h"

/// \brief Flash the leds infinitely
int main(void)
{
    tiva_setup();
    led_setup();
    led_set(LED_COLOR_GREEN);
}