# Auto-discovers test .c files in each category directory and registers
# each one as its own executable + individual CTest test.
#
# TO ADD A NEW TEST to an existing category (unit/adversarial/fuzz):
#   Just drop a new .c file in that directory. CONFIGURE_DEPENDS makes
#   most generators (Ninja, Makefiles) automatically notice the new
#   file and re-run CMake on the next build. If your generator doesn't
#   pick it up, re-run `cmake .` (or your original cmake configure
#   command) once from the build directory.
#
# TO ADD A NEW CATEGORY:
#   mkdir tests/<new_category>, then add one line below:
#       cesse_add_tests(<new_category> "<new_category>")
#
# Every test file is a standalone executable with its own main() (see
# tests/framework/test.h) and is linked against the cesse library
# (same compiler/sanitizer flags as the library itself, propagated
# via target_link_libraries since they're PUBLIC on the cesse target).

set(CESSE_ALL_TEST_TARGETS "")

function(cesse_add_tests category_dir label)
    file(GLOB test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${category_dir}/*.c")
    foreach(test_src ${test_sources})
        get_filename_component(test_name ${test_src} NAME_WE)
        set(target_name "cesse_test_${test_name}")
        add_executable(${target_name} ${test_src})
        target_link_libraries(${target_name} PRIVATE cesse)
        target_include_directories(${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
        add_test(NAME ${test_name} COMMAND ${target_name})
        set_tests_properties(${test_name} PROPERTIES LABELS "${label}" TIMEOUT 120)
        list(APPEND CESSE_ALL_TEST_TARGETS ${target_name})
    endforeach()
    set(CESSE_ALL_TEST_TARGETS "${CESSE_ALL_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

cesse_add_tests(unit "unit")
cesse_add_tests(adversarial "adversarial")
cesse_add_tests(fuzz "fuzz")

# `cmake --build <dir> --target coverage` (meaningful when the current
# preset's CESSE_COMPILE_OPTIONS includes --coverage, e.g. the "coverage"
# preset): builds every test binary above, runs the full suite, then
# produces both an lcov .info file and an HTML report under
# <build>/coverage_html/index.html. Defined whenever lcov/genhtml are
# available, regardless of preset -- running it under a preset that wasn't
# built with --coverage just produces an empty/near-empty report, which is
# lcov's own honest way of saying "there's no instrumentation to read here".
find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)
if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E rm -rf coverage_html coverage.info coverage_filtered.info
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${LCOV_EXECUTABLE} --capture --directory ${CMAKE_BINARY_DIR}
                --output-file coverage.info --ignore-errors mismatch,negative
        COMMAND ${LCOV_EXECUTABLE} --extract coverage.info "${CMAKE_SOURCE_DIR}/src/*"
                --output-file coverage_filtered.info --ignore-errors unused
        COMMAND ${GENHTML_EXECUTABLE} coverage_filtered.info --output-directory coverage_html
        COMMAND ${LCOV_EXECUTABLE} --summary coverage_filtered.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${CESSE_ALL_TEST_TARGETS}
        COMMENT "Building tests, running the suite, and generating a coverage report"
        VERBATIM
    )
else()
    message(STATUS "lcov/genhtml not found -- 'coverage' target unavailable")
endif()
