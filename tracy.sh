#!/bin/bash

cd dependencies/tracy/profiler
cmake -DCMAKE_BUILD_TYPE=Release -B build .
cmake --build build -j32
cd ../../..
./dependencies/tracy/profiler/build/tracy-profiler



