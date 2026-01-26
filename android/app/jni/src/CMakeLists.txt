cmake_minimum_required(VERSION 3.6)

project(MY_APP)

find_library(SDL2 SDL2)
find_library(SDL2_gfx SDL2_gfx)

#find_package(SDL2_gfx REQUIRED)

add_library(main SHARED
    main.c
)

target_link_libraries(main
    SDL2
    SDL2_gfx
    android
    log
)

set_target_properties(main PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
