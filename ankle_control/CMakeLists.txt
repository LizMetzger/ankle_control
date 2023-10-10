cmake_minimum_required(VERSION 3.14)
project(ankle_control VERSION 0.1.0)

find_package(nuhal REQUIRED)
find_package(cmakeme REQUIRED)
cmakeme_defaults(RelWithDebInfo)
find_package(TivaCMake REQUIRED)

add_library(ankle_control INTERFACE)
# # add all the .c files: src/${file_name}.c
# add_library(ankle_control
#     src/controller.c
#     src/encoder.c
#     src/force_sensor.c
#     src/servo.c
# )

# # add_library(ankle_control_src INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_include_directories(ankle_control INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )

target_sources(ankle_control INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/encoder.c>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/force_sensor.c>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/force_sensor.c>
)

cmakeme_install(
  TARGETS ankle_control
  NAMESPACE ankle_control
  ARCH_INDEPENDENT
  PACKAGE_NAME ankle_control
  DEPENDS nuhal
)

add_executable(exec_test
  src/exec_test.c
)
target_link_libraries(exec_test
  nuhal::nuhal
  ankle_control
)

add_library(ankle_control_tiva)
target_link_libraries(ankle_control_tiva
  PUBLIC ankle_control m nuhal::nuhal cmakeme_flags
)

cmakeme_install(TARGETS ankle_control_tiva
  NAMESPACE ankle_control
  DEPENDS
  ankle_control
  TivaCMake
)

add_executable(tiva_test src/tiva_test.c)
target_link_libraries(tiva_test nuhal::nuhal ankle_control_tiva)

  # target_include_directories(ankle_control INTERFACE
  # $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  # )
# # target_link_libraries(ankle_control ankle_control_src nuhal)# cmakeme_flags TivaCMake::driverlib m)
#   # PRIVATE nuhal cmakeme_flags 
#   # PUBLIC nuhal TivaCMake::driverlib m)

# cmakeme_install(TARGETS ankle_control driverlib NAMESPACE ankle_control DEPENDS nuhal TivaCMake) 

# # # Enable some integration tests. These are not automated because they require a microcontroller to be plugged in
# add_executable(tiva_test src/tiva_test.c)
# target_link_libraries(tiva_test nuhal cmakeme_flags) 