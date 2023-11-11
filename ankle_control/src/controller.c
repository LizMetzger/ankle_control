/// \file
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
#include "driverlib/qei.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "ankle_control/encoder.h"
#include "ankle_control/force_sensor.h"
#include "ankle_control/dynamixel_sdk/dynamixel_sdk.h"

#define BUTTON PIN('A',3)

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
    // enable reading ADC from pin E3
    adc_enable();
    // enable reading from qei for the encoder
    encoder_enable();

    // open UART communication
    const struct uart_port * port =
        uart_open("0", 1000000, UART_FLOW_NONE, UART_PARITY_NONE);

    // initialize vaiables
    const char data[] = "push \n";
    uint32_t FSR_val = 123;
    char FSR_str[12];

    uint32_t pos_val = 123;
    char pos_str[12];

    uint32_t vel_val = 123;
    char vel_str[12];

    for(;;)
    {
        // read force sensor data
        FSR_val = adc_read();
        // write it to memory
        adc_write(FSR_val, FSR_str, sizeof(FSR_str));

        pos_val = encoder_pos();
        // write it to memory
        encoder_write(pos_val, pos_str, sizeof(pos_str));

        // read encoder position
        vel_val = encoder_vel();
        // write it to memory
        encoder_write(vel_val, vel_str, sizeof(vel_str));

        if(QEIIntStatus(QEI0_BASE, true) & QEI_INTINDEX)
                {
                // led_set(LED_COLOR_RED);
                // Print a message to your console or serial output to indicate the index detection
                // const char data[] = "push \n";
                uart_write_block(port, &data, strlen(data), 0);
                // Clear the interrupt flag
                QEIIntClear(QEI0_BASE, QEI_INTINDEX);
                // time_delay_ms(1000);
                }
        //when the button is pushed
        if(pin_read(BUTTON))
        {
            led_set(LED_COLOR_GREEN);
            
            // print test message
            // uart_write_block(port, &data, strlen(data), 0);
            // print force sensor data
            // uart_write_block(port, &FSR_str, strlen(FSR_str), 0);
            // print encoder position
            // uart_write_block(port, &pos_str, strlen(pos_str), 0);
            // print encoder velocity
            // uart_write_block(port, &vel_str, strlen(vel_str), 0);
            time_delay_ms(100);
            // read encoder position
            
            uart_write_block(port, &pos_str, strlen(pos_str), 0);

        }
        else
        {
            led_set(LED_COLOR_BLUE);
        }
    }
    return 0;
}