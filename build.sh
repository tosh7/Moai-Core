rm -rf build
mkdir build
mkdir temporary

clang++ -std=c++17 \
    -target arm64-apple-ios13.0 \
    -isysroot $(xcrun --sdk iphoneos --show-sdk-path) \
    -c source/*.cpp
    
ar rcs build/libmoai.a *.o
rm -rf temporary
rm -rf *.o