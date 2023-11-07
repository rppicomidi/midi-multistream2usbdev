cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(midi2usbdev C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

set(PROJECT midi-multstream2usbdev)

project(${PROJECT})
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pio_midi_uart_lib)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/midi_uart_lib)

add_executable(${PROJECT}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/usb_descriptors.c
  ${CMAKE_CURRENT_LIST_DIR}/midi_device_multistream.c
)

target_include_directories(${PROJECT} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/preprocessor/include
)

target_link_options(${PROJECT} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${PROJECT} PRIVATE -Wall -Wextra -DCFG_TUSB_DEBUG=1)
#target_link_libraries(${PROJECT} pio_midi_uart_lib midi_uart_lib tinyusb_device tinyusb_board pico_stdlib)
target_link_libraries(${PROJECT} pio_midi_uart_lib tinyusb_device tinyusb_board pico_stdlib)

pico_add_extra_outputs(${PROJECT})
