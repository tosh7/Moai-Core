# doesn't work
# to make this work, it will need pbproj file

sh build.sh

mkdir -p moai-arm64
cp build/libmoai.a moai-arm64

mkdir -p moai-x86_64
cp build/libmoai.a moai-x86_64

xcodebuild -scheme MyLibrary -configuration Release -destination 'generic/platform=iOS' -archivePath 'archives/MyLibrary-iOS' archive
xcodebuild -scheme MyLibrary -configuration Release -destination 'generic/platform=iOS Simulator' -archivePath 'archives/MyLibrary-iOS-Simulator' archive

xcodebuild -create-xcframework -framework 'archives/MyLibrary-iOS.xcarchive/Products/Library/Frameworks/MyLibrary.framework' -framework 'archives/MyLibrary-iOS-Simulator.xcarchive/Products/Library/Frameworks/MyLibrary.framework' -output 'MyLibrary.xcframework'

xcodebuild -create-xcframework \
    -library moai-arm64/libmoai.a \
    -output build/moai.xcframework

rm -rf moai-arm64
rm -rf moai-x86_64