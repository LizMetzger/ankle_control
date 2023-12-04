#ifndef PROSTHETIC_ANKLE_H
#define PROSTHETIC_ANKLE_H

void enable_servo();

void TxOffRxOn();

void TxOnRxOff();

void UARTIntHandler();

void toggleServoLED();

void torqueEnablePacket();

void writePosPacket(int pos);

void readPosPacket();

#endif