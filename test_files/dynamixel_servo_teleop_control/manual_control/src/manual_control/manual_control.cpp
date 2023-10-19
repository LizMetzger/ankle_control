#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include <map>


#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#define STDIN_FILENO 0
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include "control/control_dyn.h"
#include "dynamixel_sdk/dynamixel_sdk.h"
#include "dynamixel_sdk/port_handler.h"
#include "dynamixel_sdk/port_handler_linux.h"

#define STDIN_FILENO 0

// define dynamixel information
#define X_SERIES
#define ADDR_TORQUE_ENABLE          64
#define ADDR_GOAL_POSITION          116
#define ADDR_PRESENT_POSITION       132
#define OPERATING_MODE              11
#define EXTENDED_POSITION           4
// #define MINIMUM_POSITION_LIMIT      -4095  // Refer to the Minimum Position Limit of product eManual
// #define MAXIMUM_POSITION_LIMIT      4095  // Refer to the Maximum Position Limit of product eManual
#define MAX_POSITION_VALUE              1048575
#define BAUDRATE                    57600
#define ADDE_PRESENT_CURRENT        126

#define PROTOCOL_VERSION  2.0

#define DEVICENAME  "/dev/ttyUSB0"

#define TORQUE_ENABLE                   1
#define TORQUE_DISABLE                  0
#define DXL_MOVING_STATUS_THRESHOLD     20  // DYNAMIXEL moving status threshold
#define ESC_ASCII_VALUE                 0x1b
#define SPACE_ASCII_VALUE               0x20

#define NUMB_OF_DYNAMIXELS              1
#define DXL_ID                          1
#define MAX_ROT                         16380
#define MIN_ROT                         0
#define FULL_ROT                        4095
#define HALF_ROT                        2048

// initialize the amount to increment the position by in degrees
float position_increment = 1.0;

// initialize a list of goal positions for each servo
// std::vector<float> servo_positions = {2027.0, 0.0, 2027.0, 0.0};
std::vector<float> servo_positions(NUMB_OF_DYNAMIXELS);

// Map for movement keys
std::map<char, std::vector<float>> moveBindings
{
  {'q', {FULL_ROT}},
  {'w', {HALF_ROT}},
  {'e', {-FULL_ROT}},
  {'r', {-HALF_ROT}},
  {'s', {0}}
};

// Map for servo controls, position of servo in servo_positions vector (add 1 for the actual servo id)
std::map<char, std::vector<float>> servoBindings
{
  {'q', {1}},
  {'w', {1}},
  {'e', {1}},
  {'r', {1}},
  {'s', {1}}
};

// Map for speed keys
std::map<char, std::vector<float>> speedBindings
{
  {'a', {1.0}},
  {'z', {-1.0}},
};

// Reminder message
const char* msg = R"(
To move servo these bindings:
---------------------------
Spin the servo clockwise with 'q' and 'w' and counterclockwise with 'e' and 'r'

a/z : increase/decrease max speeds of all servos by 10%

CTRL-C to quit
)";

char key(' ');
int dxl_id = 0;
int goal = 0;
int temp_goal = 0;
float goal_pose = 0.0;

// For non-blocking keyboard inputs
int getch(void)
{
  int ch;
  struct termios oldt;
  struct termios newt;

  // Store old settings, and copy to new settings
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;

  // Make required changes and apply the settings
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_iflag |= IGNBRK;
  newt.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
  newt.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG | IEXTEN);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;
  tcsetattr(fileno(stdin), TCSANOW, &newt);

  // Get the current character
  ch = getchar();

  // Reapply old settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  return ch;
}

int kbhit(void)
{
#if defined(__linux__) || defined(__APPLE__)
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
#elif defined(_WIN32) || defined(_WIN64)
  return _kbhit();
#endif
}

void msecSleep(int waitTime)
{
#if defined(__linux__) || defined(__APPLE__)
  usleep(waitTime * 1000);
#elif defined(_WIN32) || defined(_WIN64)
  _sleep(waitTime);
#endif
}

int main()
{
  // Initialize PortHandler instance
  // Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize PacketHandler instance
  // Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  int dxl_comm_result = COMM_TX_FAIL;             // Communication result

  uint8_t dxl_error = 0;                          // Dynamixel error
  int32_t dxl_present_position = 0;               // Present position

  // Open port
  if (portHandler->openPort())
  {
    printf("Succeeded to open the port!\n");
  }
  else
  {
    printf("Failed to open the port!\n");
    printf("Press any key to terminate...\n");
    getch();
    return 0;
  }

  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    printf("Succeeded to change the baudrate!\n");
  }
  else
  {
    printf("Failed to change the baudrate!\n");
    printf("Press any key to terminate...\n");
    getch();
    return 0;
  }

  // Set operating mode to extended position control mode
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID, OPERATING_MODE, EXTENDED_POSITION, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }
  else
  {
    printf("Operating mode changed to extended position control mode. \n");
  }


  // Enable Dynamixel Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID, ADDR_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }
  else
  {
    printf("Dynamixel has been successfully connected \n");
  }


    while(true){
        key = getch();
        // check if the pressed key is one of my bindings
        if (moveBindings.count(key)==1){
            //increment goal postion
            temp_goal = goal + moveBindings[key][0];
            if (temp_goal <= MAX_ROT and temp_goal >= MIN_ROT){
                goal = temp_goal;
            }
            // write goal position
            dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL_ID, ADDR_GOAL_POSITION, goal, &dxl_error);
        }
        // check if a speed binding was pressed
        else if (speedBindings.count(key) == 1){
            position_increment += .5 * speedBindings[key][0];
            printf("speed binding: %f \n", position_increment);
        }
        // quit if escape is pressed
        else if (key ==  ESC_ASCII_VALUE)
        {
            printf("\n\n                 .     .\n              .  |\\-^-/|  .    \n             /| } O.=.O { |\\\n\n                 CH3EERS\n\n");
            break;
        }
        // do nothing if an unbound key was pressed
        else{
            key = ' ';
        }
        do{
            // Read the Present Position
            dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, DXL_ID, ADDR_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
            if (dxl_comm_result != COMM_SUCCESS) {
                printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
            }
            else if (dxl_error != 0) {
                printf("%s\n", packetHandler->getRxPacketError(dxl_error));
            }

            printf("[ID:%03d] Goal Position:%03f  Present Position:%03f\n", DXL_ID, convert::tics2deg(goal_pose), convert::tics2deg(dxl_present_position));
        } while(dxl_present_position != goal);
    }
//   while(1)
//   {
//     printf("\nPress any key to continue! (or press ESC to quit!)\n");
//     if (getch() == ESC_ASCII_VALUE)
//       break;

//     printf("  Press SPACE key to clear multi-turn information! (or press ESC to stop!)\n");

//     // Write goal position
//     dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL_ID, ADDR_GOAL_POSITION, MAX_POSITION_VALUE, &dxl_error);
//     if (dxl_comm_result != COMM_SUCCESS)
//     {
//       printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//     }
//     else if (dxl_error != 0)
//     {
//       printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//     }

//     do
//     {
//       // Read present position
//       dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, DXL_ID, ADDR_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
//       if (dxl_comm_result != COMM_SUCCESS)
//       {
//         printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//       }
//       else if (dxl_error != 0)
//       {
//         printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//       }

//       printf("  [ID:%03d] GoalPos:%03d  PresPos:%03d\r", DXL_ID, MAX_POSITION_VALUE, dxl_present_position);

//       if (kbhit())
//       {
//         char c = getch();
//         if (c == SPACE_ASCII_VALUE)
//         {
//           printf("\n  Stop & Clear Multi-Turn Information! \n");

//           // Write the present position to the goal position to stop moving
//           dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL_ID, ADDR_GOAL_POSITION, dxl_present_position, &dxl_error);
//           if (dxl_comm_result != COMM_SUCCESS)
//           {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//           }
//           else if (dxl_error != 0)
//           {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//           }

//           msecSleep(300);

//           // Clear Multi-Turn Information
//           dxl_comm_result = packetHandler->clearMultiTurn(portHandler, DXL_ID, &dxl_error);
//           if (dxl_comm_result != COMM_SUCCESS)
//           {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//           }
//           else if (dxl_error != 0)
//           {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//           }

//           // Read present position
//           dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, DXL_ID, ADDR_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
//           if (dxl_comm_result != COMM_SUCCESS)
//           {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//           }
//           else if (dxl_error != 0)
//           {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//           }

//           printf("  Present Position has been reset. : %03d \n", dxl_present_position);

//           break;
//         }
//         else if (c == ESC_ASCII_VALUE)
//         {
//           printf("\n  Stopped!! \n");

//           // Write the present position to the goal position to stop moving
//           dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL_ID, ADDR_GOAL_POSITION, dxl_present_position, &dxl_error);
//           if (dxl_comm_result != COMM_SUCCESS)
//           {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//           }
//           else if (dxl_error != 0)
//           {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//           }

//           break;
//         }
//       }

//     }while((abs(MAX_POSITION_VALUE - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD));

//     printf("\n");
//   }

  // Disable Dynamixel Torque
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID, ADDR_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS)
  {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
  }
  else if (dxl_error != 0)
  {
    printf("%s\n", packetHandler->getRxPacketError(dxl_error));
  }

  // Close port
  portHandler->closePort();

  return 0;
}

// int main(){
//     // Initialize PortHandler instance
//     // Set the port path
//     // Get methods and members of PortHandlerLinux or PortHandlerWindows
//     dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

//     // Initialize PacketHandler instance
//     // Set the protocol version
//     // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
//     dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

//     int dxl_comm_result = COMM_TX_FAIL;             // Communication result
//     uint8_t dxl_error = 0;                          // DYNAMIXEL err

//     int32_t dxl_present_position = 0;  // Read 4 byte Position data
//     // int32_t dxl_present_current = 0;  // Read 4 byte Position data
//     int32_t last_pos = 0;

//     // Open port
//     if (portHandler->openPort()) {
//         printf("Succeeded to open the port!\n");
//     }
//     else {
//         printf("Failed to open the port!\n");
//         printf("Press any key to terminate...\n");
//         getch();
//         return 0;
//     }

//     // Set port baudrate
//     if (portHandler->setBaudRate(BAUDRATE)) {
//         printf("Succeeded to change the baudrate!\n");
//     }
//     else {
//         printf("Failed to change the baudrate!\n");
//         printf("Press any key to terminate...\n");
//         getch();
//         return 0;
//     }
    
//     // Set the DYNAMIXEL to extended position mode
//     for (int i = 1; i <= NUMB_OF_DYNAMIXELS; i++){
//         dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, i, OPERATING_MODE, EXTENDED_POSITION, &dxl_error);
//         if (dxl_comm_result != COMM_SUCCESS) {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//         }
//         else if (dxl_error != 0) {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//         }
//         else {
//             printf("Succeeded enabling extended postion mode for servo ID %d \n", i);
//         }
//     }

//     // Enable DYNAMIXEL Torque
//     for (int i = 1; i <= NUMB_OF_DYNAMIXELS; i++){
//         dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, i, ADDR_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
//         if (dxl_comm_result != COMM_SUCCESS) {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//         }
//         else if (dxl_error != 0) {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//         }
//         else {
//             printf("Succeeded enabling DYNAMIXEL Torque for servo ID %d \n", i);
//         }
//     }



//     // For each servo, add its current position to the servo pose 
//     for (int i = 1; i <= NUMB_OF_DYNAMIXELS; i++){
//         dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, i, ADDR_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
//         if (dxl_comm_result != COMM_SUCCESS) {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//         }
//         else if (dxl_error != 0) {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//         }
//         printf("[ID:%03d] Present Position:%03f\n", i, convert::tics2deg(dxl_present_position));
//         servo_positions[i-1] = convert::tics2deg(dxl_present_position);
//     }

//     std::cout << "waiting for key inputs... \n";
    // while(true){
    //     key = getch();
    //     // check if the pressed key is one of my bindings
    //     if (moveBindings.count(key)==1){
    //         // set the last position to be the current position of the servo
    //         last_pos = servo_positions.at(servoBindings[key][0]);
    //         // get the id of the dynamixel being commanded
    //         dxl_id = servoBindings[key][0] + 1;
    //         // update the servo position to be incremented by the value of the increment * the movebinding
    //         servo_positions.at(servoBindings[key][0]) += position_increment * moveBindings[key][0];
    //         // set the goal pose to be the new value
    //         goal_pose = convert::deg2tics(servo_positions.at(servoBindings[key][0]));
    //         printf("servo number: %f, servo position: %f \n", servoBindings[key][0]+1.0, servo_positions.at(servoBindings[key][0]));
    //     }
    //     // check if a speed binding was pressed
    //     else if (speedBindings.count(key) == 1){
    //         position_increment += .5 * speedBindings[key][0];
    //         printf("speed binding: %f \n", position_increment);
    //     }
    //     // quit if escape is pressed
    //     else if (key ==  ESC_ASCII_VALUE)
    //     {
    //         printf("\n\n                 .     .\n              .  |\\-^-/|  .    \n             /| } O.=.O { |\\\n\n                 CH3EERS\n\n");
    //         break;
    //     }
    //     // do nothing if an unbound key was pressed
    //     else{
    //         key = ' ';
    //     }

//         // Write goal position
//         dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, dxl_id, ADDR_GOAL_POSITION, goal_pose, &dxl_error);
//         if (dxl_comm_result != COMM_SUCCESS) {
//         printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//         }
//         else if (dxl_error != 0) {
//         printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//         }

//         int give_up = 0;


//         do {
//         // Read the Present Position
//         dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, dxl_id, ADDR_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
//         if (dxl_comm_result != COMM_SUCCESS) {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//         }
//         else if (dxl_error != 0) {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//         }

//         printf("[ID:%03d] Goal Position:%03f  Present Position:%03f\n", dxl_id, convert::tics2deg(goal_pose), convert::tics2deg(dxl_present_position));

//         // if the current present position is the same as it was last time then increment give up
//         // printf("last_pos: %d current pose: %f   " , last_pos, convert::tics2deg(dxl_present_position));
//         if (last_pos == static_cast<int>(convert::tics2deg(dxl_present_position))){
//             give_up++;
//         }
//         // set last pose to be the current position
//         last_pos = convert::tics2deg(dxl_present_position);
//         }  while((abs(goal_pose - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD) & (give_up < 3));

//         printf("Present position of each servo:\n");
//         // For each servo print its present position
//         for (int i = 1; i <= NUMB_OF_DYNAMIXELS; i++){
//             dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, i, ADDR_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
//             if (dxl_comm_result != COMM_SUCCESS) {
//                 printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//             }
//             else if (dxl_error != 0) {
//                 printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//             }
//             printf("[ID:%03d] Present Position:%03f\n", i, convert::tics2deg(dxl_present_position));
//         }
//     }

//     // Disable DYNAMIXEL Torque
//     for (int j = 1; j <= NUMB_OF_DYNAMIXELS; j++){
//         dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, j, ADDR_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
//         if (dxl_comm_result != COMM_SUCCESS) {
//             printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
//         }
//         else if (dxl_error != 0) {
//             printf("%s\n", packetHandler->getRxPacketError(dxl_error));
//         }
//         else {
//             printf("Succeeded disabling DYNAMIXEL Torque for servo id %d.\n", j);
//         }
//     }

//     // Close port
//     portHandler->closePort();
//     return 0;
// }