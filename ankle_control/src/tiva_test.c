/// \file
/// \brief Display Led's RED, GREEN, BLUE, WHITE, each for one second
/// Designed for use with the EK-TM4C123GXL board
#include "string.h"
#include "nuhal/error.h"
#include "nuhal/tiva.h"
#include "nuhal/time.h"
#include "nuhal/uart.h"
#include "nuhal/utilities.h"
#include "nuhal/uart_tiva.h"
#include "nuhal/led.h"
#include "nuhal/pin_tiva.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "ankle_control/encoder.h"
#include "ankle_control/force_sensor.h"

// #define FSR PIN('A',2)
#define BUTTON PIN('A',3)

uint32_t ADC0Value[1];

static const struct pin_configuration pins[] =
{
    // sensor peripherals
    // {FSR, PIN_INPUT},
    {BUTTON, PIN_INPUT},
    // servo
    // uart pins
    {GPIO_PA0_U0RX,  PIN_UART},
    {GPIO_PA1_U0TX, PIN_UART}
};

/// \brief Flash the leds infinitely
int main(void)
{
    tiva_setup();
    pin_setup(pins, ARRAY_LEN(pins));
    led_setup();

    // open UART communication
    const struct uart_port * port =
        uart_open("0", 1000000, UART_FLOW_NONE, UART_PARITY_NONE);

    // initialize vaiables
    const char data[] = "push \n";

    for(;;)
    {
        if(pin_read(BUTTON))
        {
            led_set(LED_COLOR_RED);
            uart_write_block(port, &data, strlen(data), 0);
            time_delay_ms(100);
        }
        else
        {
            led_set(LED_COLOR_BLUE);
        }
    }
    return 0;
}