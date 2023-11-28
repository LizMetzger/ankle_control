#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ankle_control/force_sensor.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "nuhal/uart_tiva.h"

#define SEQ 3

uint32_t FRS_val;

/// @brief enables reading data from the force sensor
void adc_enable()
{
    // enale clock to ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    // enable the clock to PORTE
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    // enable PE3 to be an analog pin
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    // Disable the sample sequencer during config
    ADCSequenceDisable(ADC0_BASE, SEQ); // clear bit3 for SS3
    // Setting a sampling option
    ADCSequenceConfigure(ADC0_BASE, SEQ, ADC_TRIGGER_PROCESSOR, 0); // processor event is the start trigger
    // // Select an analog channel
    ADCSequenceStepConfigure(ADC0_BASE, SEQ, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    // Enable the sample sequencer
    ADCSequenceEnable(ADC0_BASE, SEQ);;
    return;
}

/// @brief reads the data from the force sensor
/// @return force scaled from 0-100
uint32_t adc_read()
{
    // start ADC conversion
    ADCProcessorTrigger(ADC0_BASE, SEQ);

    while (!ADCIntStatus(ADC0_BASE, SEQ, false)) {
    // Wait for the conversion to complete
    }

    ADCSequenceDataGet(ADC0_BASE, SEQ, &FRS_val);
    float fVoltage = FRS_val / 1024.0f * 3.0f;
	unsigned ulVoltage = fVoltage * 100;

    return ulVoltage;
}

/// @brief used to write the data reading to screen
/// @param data the read force from the fsr
/// @param str the string to write the data to
/// @param size the length of the string
void adc_write(uint32_t data, char *str, size_t size)
{
    // Format the integer as a string and store it in the provided 'str' buffer.
    snprintf(str, size, "%lu \n", (unsigned long)data);
    str[size] = '\0';
    return;
}