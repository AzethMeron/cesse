cmake_minimum_required(VERSION 3.21)
project(cesse C)

# ---------------------------------------------------------------------------
# All compiler/linker configuration lives in CMakePresets.json, not here.
# This file only consumes CESSE_COMPILE_OPTIONS / CESSE_LINK_OPTIONS as
# plain cache strings and applies them -- it has no opinion about what a
# "debug" or "release" build should look like. See CMakePresets.json for
# the actual flag sets, and tests/README.md for how to add another preset.
# ---------------------------------------------------------------------------
set(CESSE_COMPILE_OPTIONS "" CACHE STRING "Space-separated compiler flags for the cesse library and every test")
set(CESSE_LINK_OPTIONS "" CACHE STRING "Space-separated linker flags for the cesse library and every test")

separate_arguments(CESSE_COMPILE_OPTIONS_LIST UNIX_COMMAND "${CESSE_COMPILE_OPTIONS}")
separate_arguments(CESSE_LINK_OPTIONS_LIST UNIX_COMMAND "${CESSE_LINK_OPTIONS}")

# ---------------------------------------------------------------------------
# The library itself. main.c is excluded on purpose: it defines its own
# main(), and every test executable below defines its own main() too --
# linking both into the same binary would be a duplicate-symbol error.
# Neither headers/ nor src/ are modified by anything in this file.
# ---------------------------------------------------------------------------
file(GLOB CESSE_LIB_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM CESSE_LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")

add_library(cesse STATIC ${CESSE_LIB_SOURCES})
target_include_directories(cesse PUBLIC "${CMAKE_SOURCE_DIR}/headers")
target_compile_options(cesse PUBLIC ${CESSE_COMPILE_OPTIONS_LIST})
target_link_options(cesse PUBLIC ${CESSE_LINK_OPTIONS_LIST})
target_link_libraries(cesse PUBLIC m)

enable_testing()
add_subdirectory(tests)
