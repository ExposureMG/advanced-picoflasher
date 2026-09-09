# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.13)

# Auto-fetch Pico SDK from git if not locatable
if(NOT DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_FETCH_FROM_GIT ON CACHE BOOL "Fetch Pico SDK if not found")
endif()

# Include build functions from Pico SDK
include(pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(PicoFlasher C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source files
add_executable(${PROJECT_NAME} 
    main.c
    usb_descriptors.c
    xbox.c
    pio_jtag.c
    cmd.c
    get_serial.c
    mode_manager.c
)

# Generate PIO header for JTAG
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/jtag.pio)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
    
# Link to pico libraries
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_spi
    hardware_pio
    hardware_dma
    hardware_sync
    hardware_flash
)

set(FLASHER_BOARD "pico" CACHE STRING "Select the board pinout to use")
set_property(CACHE FLASHER_BOARD PROPERTY STRINGS "pico" "waveshare_rp2040_zero")

if(FLASHER_BOARD STREQUAL "waveshare_rp2040_zero")
    target_compile_definitions(${PROJECT_NAME} PRIVATE WAVESHARE_RP2040_ZERO=1)
endif()

# We are not using stdio
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
