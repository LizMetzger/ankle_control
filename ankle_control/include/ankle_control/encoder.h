#ifndef PROSTHETIC_ANKLE_H
#define PROSTHETIC_ANKLE_H

#include<stdint.h>
#include<stdbool.h> 

void encoder_enable();

uint32_t encoder_pos();

uint32_t encoder_vel();

void encoder_write();

// void checsk_idx();

#endif