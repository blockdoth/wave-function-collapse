cmake_minimum_required(VERSION 3.10)

project(wave_function_collapse C)

set(CMAKE_C_STANDARD 23)

set(SOURCE_FILES main.c hashmap.h
        main2.c
        grid.c)

# Add the path to raylib library if it's not in the default system path
# set(CMAKE_PREFIX_PATH "/path/to/raylib")

find_package(raylib REQUIRED)

add_executable(wave_function_collapse ${SOURCE_FILES})

target_link_libraries(wave_function_collapse PRIVATE raylib)

target_compile_options(wave_function_collapse PRIVATE
        -Wall
        -Wextra
        -pedantic
)
