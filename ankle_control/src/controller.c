/// \file this file is a basic control loop for the dynamic impedence prosthetic ankle
// it detects when the foot of the user is off the ground and will set the spring to a 
// predetermined stiffness
#include "string.h"
#include "nuhal/error.h"
#include "nuhal/tiva.h"
#include "nuhal/time.h"
#include "nuhal/uart.h"
#include "nuhal/utilities.h"
#include "nuhal/uart_tiva.h"
#include "nuhal/led.h"
#include "nuhal/pin_tiva.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/qei.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "ankle_control/encoder.h"
#include "ankle_control/servo.h"
#include "ankle_control/force_sensor.h"

#define BUTTON PIN('A',3)
#define STEP_VAL 15
#define HIGH_GOAL 8000
#define LOW_GOAL 2000

static const struct pin_configuration pins[] =
{
    // button
    {BUTTON, PIN_INPUT},
    // uart pins
    {GPIO_PA0_U0RX,  PIN_UART},
    {GPIO_PA1_U0TX, PIN_UART}
};

int count = 0;
bool diff_step = true;

/// \brief Flash the leds infinitely
int main(void)
{
    tiva_setup();
    pin_setup(pins, ARRAY_LEN(pins));
    led_setup();
    // enable reading ADC from pin E3
    adc_enable();
    // enable reading from qei for the encoder
    encoder_enable();
    // enable UART communication with the servo
    enable_servo();
    IntMasterEnable();

    // make sure torque gets enabled
    for (int i = 0; i < 15; i++){
        torqueEnablePacket();
    }

    int goal_pos = HIGH_GOAL;

    // open UART communication
    const struct uart_port * port =
        uart_open("0", 1000000, UART_FLOW_NONE, UART_PARITY_NONE);

    // initialize vaiables
    const char data[] = "test \n";
    uint32_t FSR_val = 123;
    char FSR_str[12];

    uint32_t pos_val = 123;
    char pos_str[12];

    uint32_t vel_val = 123;
    char vel_str[12];

    for(;;)
    {
        // read force sensor data
        FSR_val = adc_read();
        // write it to memory
        adc_write(FSR_val, FSR_str, sizeof(FSR_str));

        pos_val = encoder_pos();
        // write it to memory
        encoder_write(pos_val, pos_str, sizeof(pos_str));

        // read encoder position
        vel_val = encoder_vel();
        // write it to memory
        encoder_write(vel_val, vel_str, sizeof(vel_str));

        // uart_write_block(port, &pos_str, strlen(pos_str), 0);
        uart_write_block(port, &FSR_str, strlen(FSR_str), 0);
        time_delay_ms(100);

        // check if foot is off the ground
        if (FSR_val < STEP_VAL && diff_step){
            int count = 0;
            diff_step = false;
            while (count < 5){
                // move the servo
                writePosPacket(goal_pos);
                count++;
            }
            // switch goal position
            if (goal_pos == HIGH_GOAL){
                goal_pos = LOW_GOAL;
            }
            else{
                goal_pos = HIGH_GOAL;
            }
        }
        else if (FSR_val > STEP_VAL && !diff_step){
            diff_step = true;
        }
        FSR_val = adc_read();
        //when the button is pushed
        // if(pin_read(BUTTON))
        // {   
        //     led_set(LED_COLOR_GREEN);
        //     toggleServoLED();
        //     // torqueEnablePacket();
        //     // writePosPacket();
        //     // readPosPacket();
        //     // print test message
        //     // uart_write_block(port, &data, strlen(data), 0);
        //     // print force sensor data
        //     // uart_write_block(port, &FSR_str, strlen(FSR_str), 0);
        //     // print encoder position
        //     // uart_write_block(port, &pos_str, strlen(pos_str), 0);
        //     // print encoder velocity
        //     // uart_write_block(port, &vel_str, strlen(vel_str), 0);
        //     time_delay_ms(100);
        //     // TxOnRxOff();
        // }
        // else
        // {
        //     led_set(LED_COLOR_BLUE);
        // }
    }
    return 0;
}