cmake_minimum_required(VERSION 3.6)

project(SDL2)

add_subdirectory(harfbuzz)
add_subdirectory(freetype)

add_subdirectory(SDL2)
add_subdirectory(SDL2_gfx)
add_subdirectory(SDL2_image)
add_subdirectory(SDL2_mixer)
add_subdirectory(SDL2_ttf)
add_subdirectory(src)

set_target_properties(SDL2 PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

target_compile_options(SDL2 PRIVATE -Wno-error=declaration-after-statement)
