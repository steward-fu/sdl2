cmake_minimum_required(VERSION 3.6)

project(GAME)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()

# SDL sources are in a subfolder named "SDL"
add_subdirectory(SDL)

set_target_properties(SDL2 PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

target_compile_options(SDL2 PRIVATE -Wno-error=declaration-after-statement)

# Compilation of companion libraries
#add_subdirectory(SDL_image)
#add_subdirectory(SDL_mixer)
#add_subdirectory(SDL_ttf)

# Your game and its CMakeLists.txt are in a subfolder named "src"
#add_subdirectory(src)
