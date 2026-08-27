cmake_minimum_required(VERSION 3.21)
project(cesse C)

# ---------------------------------------------------------------------------
# CESSE_ENABLE_SANITIZERS controls the one meaningful difference between the
# debug and release presets below. ON (the default, matching compile.sh
# exactly) is what every test in tests/ has actually been run and verified
# under. OFF trades that instrumentation for real optimization -- useful for
# benchmarking or just faster iteration once you trust the code, but tests
# haven't been validated against it, so treat failures under it as a signal
# to go reproduce them with sanitizers on, not as gospel on their own.
# ---------------------------------------------------------------------------
option(CESSE_ENABLE_SANITIZERS "Build with -fsanitize=address,undefined (matches compile.sh)" ON)

set(CESSE_COMPILE_OPTIONS
    -std=c2x
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wcast-qual -Wcast-align -Wstrict-prototypes -Wundef
    -Wmissing-prototypes -Wold-style-definition -Wwrite-strings
    -Wformat=2 -Wdouble-promotion -Wvla -Wpointer-arith -Wnull-dereference
    -fno-common
)
set(CESSE_LINK_OPTIONS "")

if(CESSE_ENABLE_SANITIZERS)
    list(APPEND CESSE_COMPILE_OPTIONS
        -O0 -g3 -fno-omit-frame-pointer
        -fsanitize=address,undefined -fno-sanitize-recover=all
    )
    list(APPEND CESSE_LINK_OPTIONS -fsanitize=address,undefined)
else()
    list(APPEND CESSE_COMPILE_OPTIONS -O2 -g)
endif()

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
target_compile_options(cesse PUBLIC ${CESSE_COMPILE_OPTIONS})
target_link_options(cesse PUBLIC ${CESSE_LINK_OPTIONS})
target_link_libraries(cesse PUBLIC m)

enable_testing()
add_subdirectory(tests)
