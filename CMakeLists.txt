cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(osd)

set(CMAKE_C_STANDARD 11)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(osd
	osd.c
	ssd1306.c
)

target_link_libraries(osd pico_stdlib hardware_i2c)

pico_enable_stdio_usb(osd 1) 
pico_enable_stdio_uart(osd 0) 

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(osd)

