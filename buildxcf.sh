sh build.sh

mkdir -p moai-arm64
cp build/libmoai.a moai-arm64

mkdir -p moai-x86_64
cp build/libmoai.a moai-x86_64

xcodebuild -create-xcframework \
    -library moai-arm64/libmoai.a \
    -library moai-x86_64/libmoai.a \
    -output build/moai.xcframework

rm -rf moai-arm64
rm -rf moai-x86_64