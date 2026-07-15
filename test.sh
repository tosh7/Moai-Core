rm -rf build
mkdir build

xcrun clang++ -std=c++17 \
    source/elevator.cpp \
    tests/test_elevator.cpp \
    -o build/test_runner

./build/test_runner
