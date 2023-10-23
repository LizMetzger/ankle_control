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

/// TODO: make it so that 0 is the first coil and that increasing moves it down
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
bool quit_program = false;
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
            goal = 0;
            dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, DXL_ID, ADDR_GOAL_POSITION, goal, &dxl_error);
            quit_program = true;
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

            printf("[ID:%03d] Goal Position:%03f  Present Position:%03f\n", DXL_ID, convert::tics2deg(goal), convert::tics2deg(dxl_present_position));
        } while(dxl_present_position + 5 <= goal or dxl_present_position - 5 >= goal);
        if (quit_program){
            printf("\n\n                 .     .\n              .  |\\-^-/|  .    \n             /| } O.=.O { |\\\n\n                 CH3EERS\n\n");
            break;
        }
    }

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