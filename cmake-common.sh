# Shared by build.sh and test.sh. Ninja is the generator we want: it tracks
# which headers each object read and rebuilds only what changed, where the
# old scripts cleared the build directory and started over every time.
#
# It is not required, though. Without it CMake falls back to make, and a
# fresh clone still builds without installing anything first.
generator="Unix Makefiles"
if command -v ninja >/dev/null 2>&1; then
    generator="Ninja"
fi
