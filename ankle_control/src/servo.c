#include "ankle_control/servo.h"
#include "nuhal/error.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "nuhal/uart_tiva.h"
#include "nuhal/pin_tiva.h"
#include "nuhal/tiva.h"
#include "nuhal/utilities.h"

// define pins
#define RXCONTROL PIN('C',6)
#define TXCONTROL PIN('C',7)

/// pin definitions for the servo
const struct pin_configuration servo_pin_table[] =
{
    {GPIO_PC4_U4RX, PIN_UART},
    {GPIO_PC5_U4TX, PIN_UART},
    {RXCONTROL, PIN_OUTPUT},
    {TXCONTROL, PIN_OUTPUT}
};

void enable_servo()
{
    // configure pins
    pin_setup(servo_pin_table, ARRAY_LEN(servo_pin_table));
    // enable uart peripheral
    tiva_peripheral_enable(UART4_BASE);
    // enable Tx, disable Rx
    TxOnRxOff();
    // Enable UART1
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);
    // UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 57600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    return;
}

void TxOffRxOn(){
    pin_write(RXCONTROL, 1);
    pin_write(TXCONTROL, 0);
    return;
}

void TxOnRxOff(){
    pin_write(TXCONTROL, 1);
    pin_write(RXCONTROL, 0);
    return;
}

void sendPos(){
    return;
}

void getPos(){
    return;
}