rm -rf build
mkdir build
mkdir temporary
clang++ -o temporary/main.o -c source/main.cpp
clang++ -o temporary/calculator.o -c source/calculator.cpp
ar rcs build/libmoai.a temporary/main.o temporary/calculator.o
rm -rf temporary