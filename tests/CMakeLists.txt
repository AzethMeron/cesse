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
    endforeach()
endfunction()

cesse_add_tests(unit "unit")
cesse_add_tests(adversarial "adversarial")
cesse_add_tests(fuzz "fuzz")
