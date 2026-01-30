cmake_minimum_required(VERSION 3.6)

project(SDL2_gfx)

find_library(SDL2 SDL2)

add_library(SDL2_gfx SHARED
    SDL2_framerate.c
    SDL2_gfxPrimitives.c
    SDL2_imageFilter.c
    SDL2_rotozoom.c
)

target_include_directories(SDL2_gfx PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
)

target_link_libraries(SDL2_gfx
    SDL2
    android
    log
)
