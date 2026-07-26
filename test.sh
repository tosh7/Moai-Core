rm -rf build
mkdir build

CXX="clang++"
if command -v xcrun >/dev/null 2>&1; then
    CXX="xcrun clang++"
fi

$CXX -std=c++23 -Werror=switch \
    source/elevator.cpp \
    tests/test_elevator.cpp \
    -o build/test_runner

./build/test_runner
