cmake_minimum_required(VERSION 3.13)
project(pmt C)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -pedantic-errors")
set(CMAKE_C_FLAGS_DEBUG "-g -D__PMT_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-gc-sections -s")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} src/main.c src/algorithms.h src/algorithms.c src/log.h src/log.c src/types.h src/algorithms/utils/common.h src/algorithms/utils/queue.h src/algorithms/aho_corasick.h src/algorithms/aho_corasick.c src/algorithms/boyer_moore.h src/algorithms/boyer_moore.c src/algorithms/ukkonen.h src/algorithms/ukkonen.c src/algorithms/wu_manber.h src/algorithms/wu_manber.c)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
