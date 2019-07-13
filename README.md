# YGOpen

The YGOpen project aims to create a free-as-in-freedom, cleanly-engineered Yu-Gi-Oh! Official Card Game simulator, complete with client and server support.

## Build

Clone [ocgcore-proto](https://github.com/DyXel/ocgcore-proto) to src/ocgcore-proto. This will likely be a Git submodule in the future.

### Linux

Requires CMake 3.12 or higher to generate Makefiles. Dependencies: `sdl2 sdl2_image sdl2_ttf protobuf nlohmann-json fmt`.

### Windows

You will need [Visual Studio](https://visualstudio.microsoft.com/) with the C++ toolchain and [CMake](https://cmake.org/download/) 3.12 or higher.

Set up [`vcpkg`](https://github.com/microsoft/vcpkg). Then do `vcpkg install --triplet x64-windows sdl2 sdl2-image sdl2-ttf protobuf nlohmann-json fmt` to build the needed dependencies from source.

Create a build directory. Do `cmake .. -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake` to create the solution files and then build from the Visual Studio solution files.

### macOS

You will need [Homebrew](https://brew.sh/) to get the needed dependencies and toolchains. After brew is installed, do `brew install cmake gcc sdl2 sdl2_image sdl2_ttf protobuf nlohmann-json fmt`.

Create a build directory. Do `cmake .. -DCMAKE_CXX_COMPILER=g++-9 -DCMAKE_CXX_FLAGS=-lc++` and then `make` to build with GCC9. Clang support is coming soon.

