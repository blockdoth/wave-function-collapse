cmake_minimum_required(VERSION 3.10)

project(wave_function_collapse C)

set(CMAKE_C_STANDARD 23)

# Add the path to raylib library if it's not in the default system path
# set(CMAKE_PREFIX_PATH "/path/to/raylib")

find_package(raylib REQUIRED)

add_executable(wave_function_collapse main.c)

target_link_libraries(wave_function_collapse PRIVATE raylib)

target_compile_options(wave_function_collapse PRIVATE
        -Wall
        -Wextra
        -pedantic
)
