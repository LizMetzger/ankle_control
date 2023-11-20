#include "ankle_control/servo.h"
#include "nuhal/error.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "nuhal/uart_tiva.h"

void enable_servo()
{
    // Enable UART1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    // enable the clock to PORTE
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 57600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    // configure control pins
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    return;
}

void TxOffRxOn(){
    return;
}

void TxOnRxOff(){
    return;
}

void sendPos(){
    return;
}

void getPod(){
    return;
}