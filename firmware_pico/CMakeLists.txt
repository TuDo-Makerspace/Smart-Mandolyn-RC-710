cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH "/opt/pico-sdk")
set(PICO_BOARD pico_w)

# Initialize the SDK based on PICO_SDK_PATH
# Note: This must happen before project()
include(pico_sdk_import.cmake)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(main
    main.cpp
)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main
    pico_stdlib 
    pico_cyw43_arch_lwip_poll  # Use poll or threadsafe_background, not both
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(main 0)
pico_enable_stdio_uart(main 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(main)
