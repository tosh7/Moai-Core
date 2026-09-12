cmake_minimum_required(VERSION 3.20)
project(MoaiCore LANGUAGES CXX)

# Both of these have to come before the targets: they apply to what is
# declared after them, and a target declared earlier would quietly miss out.

# A switch that has stopped covering its enum is a mistake, not a warning.
add_compile_options(-Werror=switch)

# Plain C++23. Left alone, CMake asks for the GNU dialect instead.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sources are listed rather than globbed. A glob is expanded when CMake
# configures, so a file added afterwards would be left out of the build
# until someone happened to configure again — a new .cpp belongs here.
add_library(moai STATIC
    source/calculator.cpp
    source/elevator.cpp
    source/world.cpp
)

target_include_directories(moai PUBLIC source/include)

# The tests are built for the machine doing the building and run there, so
# a cross build for the device has no use for them.
if(NOT CMAKE_CROSSCOMPILING)
    add_executable(test_runner
        tests/main.cpp
        tests/test_elevator.cpp
        tests/test_world.cpp
    )
    target_link_libraries(test_runner PRIVATE moai)
endif()
