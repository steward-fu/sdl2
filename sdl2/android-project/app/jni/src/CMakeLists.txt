cmake_minimum_required(VERSION 3.6)

project(MY_APP)

find_library(SDL2 SDL2)

add_library(main SHARED main.c)

target_link_libraries(main
    SDL2
    android
    log
)

set_target_properties(main PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
