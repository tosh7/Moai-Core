rm -rf build
mkdir build

xcrun clang++ -std=c++23 -Werror=switch \
    source/elevator.cpp \
    tests/test_elevator.cpp \
    -o build/test_runner

./build/test_runner
