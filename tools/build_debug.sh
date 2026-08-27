#!/bin/bash
cmake --preset debug && cmake --build --preset debug -j && ctest --preset debug
