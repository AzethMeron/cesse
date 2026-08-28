# cesse test suite

CMake + CTest. Nothing in `headers/` or `src/` is touched by any of this.

## Running

Via `tools/` (recommended -- matches this repo's own convention for
routine multi-step commands):

```sh
tools/build_debug.sh        # sanitizers on -- matches compile.sh, what the
                             # tests have actually been validated under
tools/build_release.sh      # optimized, sanitizers off -- for benchmarking
tools/build_coverage.sh     # gcov instrumentation, then a coverage report at
                             # build/coverage/coverage_html/index.html
```

Equivalently, by hand:

```sh
cmake --preset debug && cmake --build --preset debug -j && ctest --preset debug
cmake --preset release && cmake --build --preset release -j && ctest --preset release
cmake --preset coverage && cmake --build build/coverage --target coverage
```

`cmake --list-presets` lists all three. Each gets its own build directory
(`build/debug`, `build/release`, `build/coverage`) so switching between them
never requires a clean rebuild.

Every flag any of these builds with -- warnings, sanitizers, optimization
level, `--coverage` -- is set directly in `CMakePresets.json`, per preset.
`CMakeLists.txt` doesn't decide any of that; it just reads whatever
`CESSE_COMPILE_OPTIONS` / `CESSE_LINK_OPTIONS` the active preset set and
applies them. To add a fourth preset (say, a stricter `-Werror` variant),
add an entry to `CMakePresets.json` with its own complete flag string --
nothing in `CMakeLists.txt` needs to change. Presets don't merge or append
flag strings from an inherited preset, so each one's `CESSE_COMPILE_OPTIONS`
needs to be fully self-contained, warnings and all, not just its own delta.

Without presets, plain CMake still works, but `CESSE_COMPILE_OPTIONS` and
`CESSE_LINK_OPTIONS` default to empty -- without setting them yourself
you'd get an unoptimized, un-instrumented, warning-free build (not even
`-std=c2x`), which is rarely what you want:

```sh
cmake -S . -B build -DCESSE_COMPILE_OPTIONS="-std=c2x -O0 -g3 -fsanitize=address,undefined ..." \
                     -DCESSE_LINK_OPTIONS="-fsanitize=address,undefined"
cmake --build build -j
ctest --test-dir build
```

Presets exist precisely so you don't have to type that -- use them.

Filtering which tests run, with or without presets:

```sh
ctest --test-dir build -L unit          # just one category
ctest --test-dir build -R test_array    # just tests matching a name
ctest --test-dir build --output-on-failure
```

Every test binary links against `libcesse.a`, built with whatever the
active preset's `CESSE_COMPILE_OPTIONS` says (the `debug` preset matches
`compile.sh` exactly, sanitizers included). A test that triggers a memory
bug or UB fails automatically via its exit code -- you don't need to add
your own checks for that on top of your assertions.

## Layout

```
tests/
  framework/test.h     - the whole test framework (single header, no deps)
  unit/                - one file per module, happy-path + basic behavior
  adversarial/          - NULL args, bad input, bounds, empty-container
                          error paths, overflow, NaN, large adversarial-
                          for-quicksort inputs
  fuzz/                 - high-iteration randomized testing, driven by
                          cesse's own Rng with fixed seeds (fully
                          reproducible -- a failure gives you the same
                          repro every time, not a one-off flake)
```

## Adding a test to an existing category

Drop a new `.c` file into `tests/unit/`, `tests/adversarial/`, or
`tests/fuzz/`. That's it -- `CONFIGURE_DEPENDS` makes CMake notice it on
the next build. If your generator doesn't pick it up automatically,
re-run the `cmake -S . -B build` command once.

Each file is a standalone executable with its own `main()`:

```c
#include "framework/test.h"
#include "cesse/whatever.h"

static void test_something(void) {
        ASSERT_EQ(1 + 1, 2);
}

int main(void) {
        TEST_INIT();
        RUN(test_something);
        return TEST_REPORT();
}
```

Available assertions: `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_NE`,
`ASSERT_NULL`, `ASSERT_NOT_NULL`, `ASSERT_STR_EQ`, `ASSERT_NEAR(a, b, eps)`
(for floating point). Every one of them `return`s from the calling function
on failure -- keep test functions `static void foo(void)`, and don't use
these inside a function that needs to return a value.

## Adding a whole new category

```sh
mkdir tests/my_new_category
```

Then add one line to `tests/CMakeLists.txt`:

```cmake
cesse_add_tests(my_new_category "my_new_category")
```

## Writing a fuzz test

The pattern used throughout `tests/fuzz/`: seed one of cesse's own `Rng`
instances with a fixed literal seed, run a large number of iterations
(1e5-3e3 depending on per-iteration cost), and check an invariant every
time -- either directly (e.g. "result is within [min, max]") or against a
shadow/reference model (e.g. a plain array mirroring what a `Stack` should
contain). Use a *second*, independently-seeded `Rng` for generating test
*parameters* (like random min/max pairs) when you don't want them
correlated with the values being tested for correctness.

Fixed seeds are deliberate: a fuzz failure should reproduce identically
every time you re-run it, not depend on wall-clock time or `/dev/urandom`.
If you want to also fuzz with a fresh, non-reproducible seed occasionally
(e.g. in CI on a schedule), add a *separate* test for that rather than
making the deterministic one non-deterministic.
