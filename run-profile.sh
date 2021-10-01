#!/usr/bin/env bash

mkdir -p profile

BRANCH=`git rev-parse --abbrev-ref HEAD`
SHA=`git rev-parse HEAD`
DATETIME=`date "+%y-%m-%d-%H-%M-%S"`
PROFILE_DIR="profile/${BRANCH}_${DATETIME}"

mkdir $PROFILE_DIR

cmake -S . -B build/valgrind -DCMAKE_BUILD_TYPE=DEBUG -DVALGRIND=1
cmake --build build/valgrind
cd build/valgrind
valgrind --leak-check=yes --log-file=../../$PROFILE_DIR/valgrind.txt ./tree --run-benchmarks 10000
cd ../..

cmake -S . -B build/gprof -DCMAKE_BUILD_TYPE=RELEASE -DGPROF=1
cmake --build build/gprof
cd build/gprof
./tree --run-benchmarks 20000000
gprof tree gmon.out > ../../$PROFILE_DIR/gprof.txt
cd ../..
