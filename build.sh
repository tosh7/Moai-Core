set -e
. ./cmake-common.sh

# CMake locates the device SDK itself, so no path is written down here.
cmake -S . -B build/ios -G "$generator" \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 >/dev/null
cmake --build build/ios

echo "built build/ios/libmoai.a"
