cmake_minimum_required(VERSION 3.13)
project(pmt C)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -pedantic-errors")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -s")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-gc-sections")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} src/main.c)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
