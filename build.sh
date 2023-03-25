rm -rf build
mkdir build
mkdir temporary

clang++ -std=c++17 \
    -target arm64-apple-ios13.0 \
    -isysroot $(xcrun --sdk iphoneos --show-sdk-path) \
    -o temporary/main.o -c source/main.cpp

clang++ -std=c++17 \
    -target arm64-apple-ios13.0 \
    -isysroot $(xcrun --sdk iphoneos --show-sdk-path) \
    -o temporary/calculator.o -c source/calculator.cpp
    
ar rcs build/libmoai.a temporary/main.o temporary/calculator.o
rm -rf temporary