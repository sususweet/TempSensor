cmake_minimum_required(VERSION 3.8)
project(tempsensor)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(./Inc ./Drivers/CMSIS/Device/ST/STM32L4xx/Include ./Drivers/STM32L4xx_HAL_Driver/Inc
        ./Drivers/BSP/STM32L4xx_Nucleo ./Drivers/BSP/Components/Common ./Drivers/CMSIS/Include)

set(SOURCE_FILES
        Inc/stm32l4xx_hal_conf.h
        Inc/stm32l4xx_it.h
        Src/main.c
        Inc/main.h
        Src/stm32l4xx_it.c
        Src/system_stm32l4xx.c
        Src/common.c
        Inc/common.h
        Src/STM32_DHT12.c
        Inc/STM32_DHT12.h
        Inc/STM32_LCD1602.h
        Src/STM32_LCD1602.c
        Inc/STM32_PMS7003.h
        Src/STM32_PMS7003.c
        Inc/STM32_ESP8266.h
        Src/stm32l4xx_hal_msp.c Src/STM32_ESP8266.c)

add_executable(tempsensor ${SOURCE_FILES})