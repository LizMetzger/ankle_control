#include "ankle_control/servo.h"
#include "nuhal/error.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "inc/tm4c123gh6pm.h"
#include "nuhal/uart_tiva.h"
#include "nuhal/pin_tiva.h"
#include "nuhal/tiva.h"
#include "nuhal/utilities.h"


// define pins
#define RXCONTROL PIN('C',6)
#define TXCONTROL PIN('C',7)

// define UART values
#define UART_NAME '4'

// define packet values
#define H1 0xFF
#define H2 0xFF
#define H3 0xFD
#define RSRV 0x00

// set the servo baud rate
#define baud 115200
// set the servo parity
#define parity UART_CONFIG_PAR_NONE

// create port stuct
// list of the ports that can be opened
// static const struct uart_port ports[] = {
//     {UART4_BASE, SYSCTL_PERIPH_UART4, INT_UART4}
// };

/// pin definitions for the servo
const struct pin_configuration servo_pin_table[] =
{
    {GPIO_PC4_U4RX, PIN_UART},
    {GPIO_PC5_U4TX, PIN_UART},
    {RXCONTROL, PIN_OUTPUT},
    {TXCONTROL, PIN_OUTPUT}
};

// void UART4_Transmitter(char data)  
// {
//     while((UART4->FR & 0x20) != 0); /* wait until Tx buffer not full */
//     UART4->DR = data;                  /* before giving it another byte */
// }

// // function to write to the servo
// void write_packet(uint16_t id, uint16_t address, uint16_t *data, uint16_t length){
//     // uart4_port.base = UART4_BASE;
//     UART5_Transmitter(*UART4_BASE, H1, sizeof(H1));
// }

/// @brief  enable the UART communication and configure it for the servo
void enable_servo()
{
    // configure pins
    pin_setup(servo_pin_table, ARRAY_LEN(servo_pin_table));
    // enable uart peripheral
    tiva_peripheral_enable(SYSCTL_PERIPH_UART4);
    // disable UART to configure
    UARTDisable(UART4_BASE);
    // configure for a baud of 115200
    UARTConfigSetExpClk(UART4_BASE, tiva_clock_hz(), baud,UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE  | parity);
    // enable the uart fifos
    UARTFIFOEnable(UART4_BASE);
    // flush the uart to eliminate any spurious data
    // time for a single character to arrive, in us, assuming 10 bits
    // per byte (byte + 1 stop and 1 start bit)
    const uint32_t byte_us = 10000000/baud;
    time_delay_us(byte_us);
    while(UARTCharsAvail(UART4_BASE))
    {
        (void)UARTCharGetNonBlocking(UART4_BASE);
        time_delay_us(byte_us);
    }
    UARTRxErrorClear(UART4_BASE);
    // uart_open('4', baud, UART_FLOW_NONE, parity);
    // enable the UART
    UARTEnable(UART4_BASE);
    // enable UART interrupts for reading
    UARTIntEnable(UART4_BASE, UART_INT_RX);
    // enable Tx, disable Rx
    TxOnRxOff();
    return;
}

/// @brief  this function switchs Tx (write) off and Rx (read) on
void TxOffRxOn(){
    pin_write(RXCONTROL, 1);
    pin_write(TXCONTROL, 0);
    return;
}

/// @brief  this function switchs Tx (write) on and Rx (read) off
void TxOnRxOff(){
    pin_write(TXCONTROL, 1);
    pin_write(RXCONTROL, 0);
    return;
}

void UARTIntHandler(void){
    // uint32_t ui32Status;
    // // Get the interrrupt status.
    // ui32Status = UARTIntStatus(UART0_BASE, true);
    // // Clear the asserted interrupts.
    // UARTIntClear(UART0_BASE, ui32Status);
    // // Loop while there are characters in the receive FIFO.
    // while(UARTCharsAvail(UART0_BASE))
    // {
    //     // Read the next character from the UART and write it back to the UART.
    //     UARTCharPutNonBlocking(UART0_BASE,
    //                                UARTCharGetNonBlocking(UART0_BASE));
    // }
}

void writeServo(const uint8_t *buffer, uint32_t size){
    // do something to send a packet
    while(size--){
        UARTCharPutNonBlocking(UART4_BASE, *buffer++);
    }
    // switch to read mode
    TxOffRxOn();
    return;
}

void readServo(){
    // do something to read a packet
    // switch to write mode
    TxOnRxOff();
    return;
}