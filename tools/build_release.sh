#!/bin/bash
cmake --preset release && cmake --build --preset release -j && ctest --preset release
