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
#include <string.h>
#include <stdio.h>

// define pins
#define RXCONTROL PIN('C',6)
#define TXCONTROL PIN('C',7)

// define UART values
#define UART_NAME '4'
#define BASE UART4_BASE

// set the servo baud rate
#define baud 115200
// set the servo parity
#define parity UART_CONFIG_PAR_NONE
// set packet values
unsigned short H1 = 0xFF;
unsigned short H2 = 0xFF;
unsigned short H3 = 0xFD;
unsigned short RSRV = 0x00;
unsigned short ID = 0x01;
unsigned short LEN1 = 0x09;
unsigned short MODE = 0x03;
unsigned short P1 = 0x74;
unsigned short P4 = 0x02;
unsigned short CRC1 = 0xCA;
unsigned short CRC2 = 0x89;

/// pin definitions for the servo
const struct pin_configuration servo_pin_table[] =
{
    {GPIO_PC4_U4RX, PIN_UART},
    {GPIO_PC5_U4TX, PIN_UART},
    {RXCONTROL, PIN_OUTPUT},
    {TXCONTROL, PIN_OUTPUT}
};

// dynamixel function to figure ou what the CRC is
unsigned short update_crc(unsigned short crc_accum, unsigned char *data_blk_ptr, unsigned short data_blk_size)
{
    unsigned short i, j;
    unsigned short crc_table[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
    };

    for(j = 0; j < data_blk_size; j++)
    {
        i = ((unsigned short)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
    }

    return crc_accum;
}

/// @brief  enable the UART communication and configure it for the servo
void enable_servo()
{
    // configure pins
    pin_setup(servo_pin_table, ARRAY_LEN(servo_pin_table));
    // enable uart peripheral
    tiva_peripheral_enable(SYSCTL_PERIPH_UART4);
    // disable UART to configure
    UARTDisable(BASE);
    // configure for a baud of 115200
    UARTConfigSetExpClk(BASE, tiva_clock_hz(), baud, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE  | parity);
    // enable the uart fifos
    UARTFIFOEnable(BASE);
    // flush the uart to eliminate any spurious data
    // time for a single character to arrive, in us, assuming 10 bits
    // per byte (byte + 1 stop and 1 start bit)
    const uint32_t byte_us = 10000000/baud;
    time_delay_us(byte_us);
    while(UARTCharsAvail(BASE))
    {
        (void)UARTCharGetNonBlocking(BASE);
        time_delay_us(byte_us);
    }
    UARTRxErrorClear(BASE);
    // uart_open('4', baud, UART_FLOW_NONE, parity);
    // enable the UART
    UARTEnable(BASE);
    // enable UART interrupts for reading
    UARTIntEnable(BASE, UART_INT_RX);
    // disable Tx, enable Rx
    TxOffRxOn();
    return;
}

/// @brief  this function switchs Tx (write) off and Rx (read) on
void TxOffRxOn(){
    pin_write(RXCONTROL, 1);
    pin_write(TXCONTROL, 0);
    time_delay_ms(10);
    return;
}

/// @brief  this function switchs Tx (write) on and Rx (read) off
void TxOnRxOff(){
    pin_write(TXCONTROL, 1);
    pin_write(RXCONTROL, 0);
    // time_delay_ms(10);
    return;
}

void UARTIntHandler(){
    const struct uart_port * port =
       uart_open("0", 1000000, UART_FLOW_NONE, UART_PARITY_NONE);
    char crcl_str[50];
   // snprintf(crcl_str, 50, "CRC: 0x%02X\n", CRC);
    char getChar;
    uint32_t ui32Status;
    // Get the interrrupt status.
    // ui32Status = UARTIntStatus(BASE, true);
    // Clear the asserted interrupts.
    // UARTIntClear(BASE, ui32Status);
    // Loop while there are characters in the receive FIFO.
    while(UARTCharsAvail(BASE))
    {
        // Read the next character from the UART and write it back to the UART.
        getChar = UARTCharGetNonBlocking(BASE);
        snprintf(crcl_str, 50, "CRCL: 0x%02X\nCRCH: 0x%02X\n", getChar);
        crcl_str[49] = '\0';
        uart_write_block(port, crcl_str, strlen(crcl_str), 0);
    }
}

void writeByteServo(unsigned char byte){
    // wait for there to be space to write
    while (UARTSpaceAvail(BASE) == 0) {};
    // write the byte
    UARTCharPut(BASE, byte);
    return;
}

void toggleServoLED(){
    // turn Tx on
    TxOnRxOff();
    // time_delay_ms(500);
    // Calculate CRCL and CRCH with dynamixel CRC code
    uint8_t CRCL;
    uint8_t CRCH;
    unsigned char packet[] = {H1, H2, H3, RSRV, ID, 0x06, 0x00, 0x03, 0x41, 0x00, 0x01, CRCL, CRCH};
    size_t packet_length = sizeof(packet) / sizeof(packet[0]);
    unsigned short CRC = update_crc(0, packet, packet_length - 2);
    CRCL = (CRC & 0x00FF);
    CRCH = ((CRC >> 8) & 0x00FF);
    // set these packet values
    packet[packet_length - 2] = CRCL;
    packet[packet_length - 1] = CRCH;
    // print the CRC
    const struct uart_port * port =
    uart_open("0", 1000000, UART_FLOW_NONE, UART_PARITY_NONE);
    char crcl_str[50];
    snprintf(crcl_str, 50, "CRCL: 0x%02X\nCRCH: 0x%02X\n", CRCL, CRCH);
    crcl_str[49] = '\0';
    uart_write_block(port, crcl_str, strlen(crcl_str), 0);
    // write each packet member to the servo
    for (size_t i = 0; i < packet_length; i++) {
        writeByteServo(packet[i]);
    }
    // disable interrupts
    UARTIntDisable(BASE, UART_INT_RX); //  interrupt here may cause a delay longer than the return delay time 
    while (UARTSpaceAvail(BASE) == 0){};
    time_delay_ms(500);
    // turn Tx off and Rx on
    TxOffRxOn();
    // enable interrupts
    UARTIntEnable(BASE, UART_INT_RX);
    return;
}

void torqueEnablePacket(){
    TxOnRxOff();
    uint8_t CRCL;
    uint8_t CRCH;
    unsigned char packet[] = {H1, H2, H3, RSRV, ID, 0x06, 0x00, 0x03, 0x40, 0x00, 0x01, CRCL, CRCH};
    unsigned short CRC = update_crc(0, packet, 11);
    CRCL = (CRC & 0x00FF);
    CRCH = ((CRC >> 8) & 0x00FF);
    writeByteServo(H1);
    writeByteServo(H2);
    writeByteServo(H3);
    writeByteServo(RSRV);
    writeByteServo(ID);
    writeByteServo(0x06); // length 1 (length = 3 + numb of params)
    writeByteServo(0x00); // length 2
    writeByteServo(0x03); // instruction
    writeByteServo(0x40); // P1
    writeByteServo(0x00); // P2
    writeByteServo(0x01); // P3
    writeByteServo(CRCL);
    UARTIntDisable(BASE, UART_INT_RX);
    writeByteServo(CRCH);
    while (UARTSpaceAvail(BASE) == 0){};
    time_delay_ms(500);
    TxOffRxOn();
    UARTIntEnable(BASE, UART_INT_RX);
    return;
}

void writePosPacket(){
    TxOnRxOff();
    uint8_t CRCL;
    uint8_t CRCH;
    unsigned char packet2[] = {H1, H2, H3, RSRV, ID, 0x09, 0x00, 0x03, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, CRCL, CRCH};
    unsigned short CRC = update_crc(0, packet2, 14);
    CRCL = (CRC & 0x00FF);
    CRCH = ((CRC >> 8) & 0x00FF);
    // for each field in the packet get a byte and write it
    writeByteServo(H1);
    writeByteServo(H2);
    writeByteServo(H3);
    writeByteServo(RSRV);
    writeByteServo(ID);
    writeByteServo(0x09);
    writeByteServo(0x00);
    writeByteServo(0x03);
    writeByteServo(0x74);
    writeByteServo(0x00);
    writeByteServo(0x00);
    writeByteServo(0x00);
    writeByteServo(0x00);
    writeByteServo(0x00);
    writeByteServo(CRCL);
    UARTIntDisable(BASE, UART_INT_RX);
    writeByteServo(CRCH);
    while (UARTSpaceAvail(BASE) == 0){};
    time_delay_ms(500);
    TxOffRxOn();
    UARTIntEnable(BASE, UART_INT_RX);
    return;
}

void readPosPacket(){
    TxOnRxOff();
    uint8_t CRCL;
    uint8_t CRCH;
    unsigned char packet2[] = {H1, H2, H3, RSRV, ID, 0x07, 0x00, 0x02, 0x84, 0x00, 0x04, 0x00, CRCL, CRCH};
    unsigned short CRC = update_crc(0, packet2, 12);
    CRCL = (CRC & 0x00FF);
    CRCH = ((CRC >> 8) & 0x00FF);
    // for each field in the packet get a byte and write it
    writeByteServo(H1);
    writeByteServo(H2);
    writeByteServo(H3);
    writeByteServo(RSRV);
    writeByteServo(ID);
    writeByteServo(0x07);
    writeByteServo(0x00);
    writeByteServo(0x02);
    writeByteServo(0x84);
    writeByteServo(0x00);
    writeByteServo(0x04);
    writeByteServo(0x00);
    writeByteServo(CRCL);
    UARTIntDisable(BASE, UART_INT_RX);
    writeByteServo(CRCH);
    while (UARTSpaceAvail(BASE) == 0){};
    TxOffRxOn();
    UARTIntEnable(BASE, UART_INT_RX);
    return;
}

void writeIntServo(int data){
    // do something to send a packet
    
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
