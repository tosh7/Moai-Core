set -e
. ./cmake-common.sh

cmake -S . -B build/host -G "$generator" >/dev/null
cmake --build build/host

./build/host/test_runner
