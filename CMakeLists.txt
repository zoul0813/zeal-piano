cmake_minimum_required(VERSION 3.16)

# Application Version
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)
set(APP_VERSION_PATCH 0)
set(APP_NAME "Zeal Piano")
set(target_name "piano")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Ensure dirs exist
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

zos_use(zvb)
find_package(ZVB_SDK REQUIRED)

zos_use(zgdk)
find_package(ZGDK REQUIRED)

project(${target_name} C ASM)

# include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories($ENV{ZVB_SDK_PATH}/include)

add_executable(${target_name}
    "src/piano.c"
    "src/file.c"
    "src/assets.c"
)

# target_link_libraries(${target_name} PRIVATE )
target_link_libraries(${target_name} PRIVATE zvb_gfx zvb_sound zgdk)
# zos_link_libraries(${target_name} PRIVATE common keyboard windows)

target_compile_options(${target_name} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:--nostdlib>
)

# -------------------------------
# Assets
# -------------------------------

gif2zeal(${target_name} FILES
    "assets/piano.gif"
    COMPRESSED)

tiled2zeal(${target_name} FILES
    "assets/piano.tmx"
    VERBOSE
    LAYER 0)

zos_add_outputs(${target_name})