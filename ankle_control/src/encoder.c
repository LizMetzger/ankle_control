#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ankle_control/encoder.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/qei.h"
#include "driverlib/uart.h"
#include "nuhal/uart_tiva.h"
#include "nuhal/pin_tiva.h"
#include "nuhal/tiva.h"
#include "nuhal/utilities.h"

/// pin definitions for the encoder
const struct pin_configuration qei_pin_table[] =
{
    {GPIO_PD6_PHA0, PIN_QEI},
    {GPIO_PD7_PHB0, PIN_QEI},
};

/// @brief configures the Tiva to read data from the encoder via QEI
void encoder_enable()
{
	// SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    // Set the value of the max number of pulses per revolution
    uint32_t ui32_MAX_PULSES = 8192;
	uint32_t ui32_VEL_PERIOD = 32;

	 // Enable GPIO Clock
	pin_setup(qei_pin_table, ARRAY_LEN(qei_pin_table));
	tiva_peripheral_enable(SYSCTL_PERIPH_QEI0);

	QEIDisable(QEI0_BASE);
	QEIIntDisable(QEI0_BASE, QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);
	QEIConfigure(QEI0_BASE, QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_RESET_IDX |
	QEI_CONFIG_QUADRATURE| QEI_CONFIG_NO_SWAP, ui32_MAX_PULSES);

	QEIFilterEnable(QEI0_BASE);
	// Enables the QEI module
	QEIEnable(QEI0_BASE);
    // Enable interrupts 
    QEIIntEnable(QEI0_BASE, QEI_INTDIR | QEI_INTINDEX);
    return;
}

/// @brief function to get the current position of the ankle
/// @return the position according to the encoder
uint32_t encoder_pos(){
    // read encoder position
    return QEIPositionGet(QEI0_BASE);
}

/// @brief function o get the velcoity that the ankle is rotatin
/// @return the velocity of the joint
uint32_t encoder_vel(){
    // read number of pulses per time period
    return QEIVelocityGet(QEI0_BASE);
}

/// @brief write the data to memory so it can be printed to screen
/// @param data the position or velocity data to be written
/// @param str the string to write it into
/// @param size the size of the data being written
void encoder_write(uint32_t data, char *str, size_t size)
{
    // Format the integer as a string and store it in the provided 'str' buffer.
    snprintf(str, size, "%ld \n", (long)data);
    str[size] = '\0';
    return;
}

// void check_idx(){
//     const struct uart_port * port =
//         uart_open("0", 1000000, UART_FLOW_NONE, UART_PARITY_NONE);
    
// }
// /// @brief reset the zero position of the ankle
// void home(){
//     return;
// }
