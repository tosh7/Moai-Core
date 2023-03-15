rm -rf build
rm -rf temporary
mkdir build
mkdir temporary
clang++ -o temporary/main.o -c source/main.cpp
ar rcs build/libmoai.a temporary/main.o
rm -rf temporary