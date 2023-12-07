# Dynamic Impedence Prosthetic Ankle Package

## Overview
This package is designed to control and test a dynamic impedance ankle that has a Dynamixel servo, quadrature rotary encoder, and force resistive sensor using a Tiva C Launchpad microcontroller. It also contains test data from measuring the stiffness of the linear spring component and the entire ankle assembly in an Instron machine.

## ankle_control
This section of the package is responsible for controlling the ankle assembly.

- `encoder.c`: Provides functions to enable the quadrature encoder with a Tiva Launchpad, read the position and velocity of the encoder, and write this information to UART.
- `force_sensor.c`: Provides functions that enable reading ADC from the Tiva's GPIO pins, read the ADC value and convert it to units of force, and write this information over UART.
- `servo.c`: Provides functions to enable UART for writing to a Dynamixel servo using Protocol 2.0, enable writing a goal position, enable reading a current position, toggle the built-in LED, and toggle between Rx and Tx modes.
- `controller.c`: Contains the main loop that, when loaded onto a Tiva, continuously reads the force sensor and encoder data to write goal positions to the servo between steps.
- `tiva_test.c`: An auxiliary file that contains a function to test if a Tiva board is working correctly. When this code is written to the board, the LED should start blue and turn red when a button is pushed.
- `exec_test.c`: An auxiliary file to test if the `CMakeLists.txt` is compiling correctly. If errors are thrown regarding this file, it indicates issues with the CMake process.


To set up a Tiva C Launchpad to run this code you can follow these instructions (you will need access to the omnid repository):
```bash
mkdir -p tiva/src
cd tiva
mkdir install
cd src
git clone git@github.com:omnid/cmakeme.git
cd cmakeme
cmake -B build . -DCMAKE_INSTALL_PREFIX=../../install/ -DCMAKE_PREFIX_PATH=../../install/
cmake --build build --target install
cd ..
git clone git@github.com:omnid/tiva_cmake.git
cd tiva_cmake
cmake -B build . -DCMAKE_INSTALL_PREFIX=../../install/ -DCMAKE_PREFIX_PATH=../../install/
cmake --build build --target install
cd ..
git clone ${my_package}
cd ${my_package/dir_where_cmakelists_is}
cmake -B build . -DCMAKE_INSTALL_PREFIX=../../../install/ -DCMAKE_PREFIX_PATH=../../../install/
# (Make sure the above relative path to install dir is correct)
cmake --build build --target install
# (This last install command is unnecessary if no further packages will depend on this package)
cmake --build build --target ${my_executable}.write
# If you get a LIBUSB_ERROR_ACCESS try running this command:
sudo chmod -R 777 /dev/bus/usb/
```


## Test Files
This section contains useful information that was either obtained during testing or used in tests.


- `dynamixel_servo_teleop_control`: Contains the necessary code to manually control the position of a Dynamixel servo using the Dynamixel SDK. To run this code, connect to the Dynamixel servo and `cd` to the `linux64` file. In this file, run these commands:

```bash
make clean
make
./manual_control
```
from here you should be able to use 'Q' and 'E' on the keyboard for full turns of the servo and 'W' and 'R' for half turns of the servo.
- `foot_data`: contains all of the raw data and an excel spreadsheet of analyzed data from conducting force testing on the entire ankle assembly with an Instrom machine. 
- `linear_data`: contains all of the raw data and excel spreadsheets of analyzed data from force testing with the linear component. This was conducted twice on different days and mechanical changes were made between these tests. The daat from the second test is what was used officially.
- `linear_component_test.ipynb`: is a notebook that displays the theoretical data and further analyzed data from the second day of linear testing. It outlines how the data was collected and compares the experimental results to the theotetical expected values. There is also data from low resolution force testing, done by hand that was not used beyond an intialy proof of concept.
- `pictures`: is a folder containing photos that are relevant to understaning the device design, the assembly of the prosthetic, and testing protocals. 
- `spring_calculations.py`: is a file that finds springs with specs that allow them to be theoretically usable for this urpose. It checks if the spring would be able to reach both the biological minum and maximum resistance based on the number of coils, diameter of the wire, diameter of the spring, and the material.


(1) Extended Tiva Instructions
1. Download Code Composer Studio from here: https://www.ti.com/tool/CCSTUDIO
2. Extract the tar.gz file
3. Run the “ccs_setup_12.3.0.00005.run” file
4. In the installer, there will be a page for checking dependencies; click on the FAQ page; scroll down to the bottom of the page to find a command for installing the dependencies
5. Run “chmod a+x uniflash.sh” in /ti/ccs1250/ccs/ccs_base/scripting/examples/uniflash/cmdLine
6. Install GCC toolchain `sudo apt install  gcc-arm-none-eabi libnewlib-arm-none-eabi gdb-multiarch`
8. Install the following TI tool to /opt directory: https://www.ti.com/tool/ARM-CGT
9. Follow the “Flashing the Firmware” or the “Template Mode” sections to flash the LED example

If you are continuing this project see [this link](https://docs.google.com/document/d/1PWttoiP_I-D5uhp3z9gbeUz3KAqcI5_J4W16wc22g_Q/edit?usp=sharing) for a list of further improvements as well as a bunch of information on choices that were made and details of the project.