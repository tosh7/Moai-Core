sh build.sh

mkdir -p moai-arm64
cp libmoai-arm64.a moai-arm64/

mkdir -p moai-x86_64
cp libmoai-x86_64.a moai-x86_64/

xcodebuild -create-xcframework \
    -library moai-arm64/libmoai-arm64.a \
    -library moai-x86_6/libmoai-x86_64.a \
    -output moai.xcframework \