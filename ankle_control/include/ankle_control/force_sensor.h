#ifndef PROSTHETIC_ANKLE_H
#define PROSTHETIC_ANKLE_H

#include<stdint.h>
#include<stdbool.h> 

void adc_enable(void);

uint32_t adc_read(void);

void adc_write();

#endif