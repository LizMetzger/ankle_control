#include <stdint.h>
#include <stdbool.h> 
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


/// @brief configures the Tiva to read data from the encoder via QEI
void encoder_enable()
{
    // Set the value of the max number of pulses per revolution
    uint32_t ui32_MAX_PULSES = 4096;
	uint32_t ui32_VEL_PERIOD = 32;

    // Enable GPIO Clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	// this loop waits for the peripheral to be ready (3 clock cycles)
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)){}

    // Enable QEI0 clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
	// this loop waits for the peripheral to be ready
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_QEI0)){}

    //Configure GPIO pins to act as QEI pins
	//GPIO PD3 -> IDX0, PD6 -> PhA0, PD7 -> PhB0
	GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PD3_IDX0);
	GPIOPinConfigure(GPIO_PD6_PHA0);
	GPIOPinConfigure(GPIO_PD7_PHB0);

    QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_RESET_IDX |
	QEI_CONFIG_CLOCK_DIR | QEI_CONFIG_NO_SWAP), ui32_MAX_PULSES);

	QEIFilterEnable(QEI0_BASE);
	//Enables the QEI module
	QEIEnable(QEI0_BASE);
	//Configure the velocity capture module
	//QEI_VELDIV_1: no predivision
	//ui32_VEL_PERIOD: number of clock ticks over which to count pulses
	QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_1,  ui32_VEL_PERIOD);
	//Enable the Velocity capture module
	QEIVelocityEnable(QEI0_BASE);
    // Enable interrupts 
    QEIIntEnable(QEI0_BASE, QEI_INTINDEX);
    return;
}

/// @brief function to get the current position of the ankle
/// @return the position according to the encoder
uint32_t encoder_pos(){
    // read encoder position
    // QEIPositionGet(QEI0_BASE);
    return QEIPositionGet(QEI0_BASE);
}   

/// @brief function o get the velcoity that the ankle is rotatin
/// @return the velocity of the joint
uint32_t encoder_vel(){
    // read number of pulses per time period
    // QEIVelocityGet(QEI0_BASE);
    return QEIVelocityGet(QEI0_BASE);
}

/// @brief write the data to memory so it can be printed to screen
/// @param data the position or velocity data to be written
/// @param str the string to write it into
/// @param size the size of the data being written
void encoder_write(uint32_t data, char *str, size_t size)
{
    // Format the integer as a string and store it in the provided 'str' buffer.
    snprintf(str, size, "%lu \n", (unsigned long)data);
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
