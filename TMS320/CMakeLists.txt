cmake_minimum_required(VERSION 3.8)
project(pm25_scanner)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(../assets/F2837xS_common/include ../assets/F2837xS_headers/include)

set(SOURCE_FILES
        F28377_LCD12864.c
        F28377_LCD12864.h
        F28377_Keyboard.c
        F28377_Keyboard.h
        main.c
        nmea0183.c
        nmea0183.h
        F28377_PM25Sensor.h
        F28377_PM25Sensor.c
        F28377_TempSensor.h
        F28377_TempSensor.c
        F28377_ESP8266_UART.h
        F28377_ESP8266_UART.c
        F28377_PM25_10_UART.h
        F28377_PM25_10_UART.c
        F28377_ATGM336H_UART.c
        F28377_ATGM336H_UART.h
        F28377_Common.h F28377_Common.c)

add_executable(pm25_scanner ${SOURCE_FILES})