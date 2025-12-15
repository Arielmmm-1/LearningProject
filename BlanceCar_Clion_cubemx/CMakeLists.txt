cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME tese1)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}

        Core/Src/motor.c
        Core/Inc/motor.h
        Core/Src/encoder.c
        Core/Inc/encoder.h
        Core/Src/usart.c
        Core/Inc/usart.h
        Core/Inc/pid.h
        Core/Src/pid.c

)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
)

# ===================== 源文件配置（保留原有内容） =====================
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # 自定义源文件
        Core/Src/IIC.c
        Core/Src/inv_mpu.c
        Core/Src/inv_mpu_dmp_motion_driver.c
        Core/Src/mpu6050.c
        Core/Src/oled.c
        Core/Src/tim.c
        Core/Src/Sr04.c
        # !!!!TIM相关HAL源文件（解决函数未定义）
        Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c
        Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c
        Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Core/Inc    Core/MyCode
        Drivers/STM32F1xx_HAL_Driver/Inc
        Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
        Drivers/CMSIS/Device/ST/STM32F1xx/Include
        Drivers/CMSIS/Include

)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        STM32F103xB
        USE_HAL_DRIVER
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        m # 数学库（软件浮点运算依赖，保留）
)

# ===================== 必须添加：生成hex/bin文件，确保烧录的是最新文件 =====================
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.bin
)