rm -rf build
mkdir build

CXX="clang++"
if command -v xcrun >/dev/null 2>&1; then
    CXX="xcrun clang++"
fi

# source/world.cpp and tests/test_world.cpp join this list once World exists.
$CXX -std=c++23 -Werror=switch -Isource/include \
    source/elevator.cpp \
    tests/test_elevator.cpp \
    tests/main.cpp \
    -o build/test_runner

./build/test_runner
