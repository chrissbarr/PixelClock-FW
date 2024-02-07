# PixelClock-FW
## Overview

## Building
### Embedded - Platformio


### Desktop - CMake
#### Windows - CMake + Ninja + MSVC
1. Open x64 Visual Studio Command Prompt (e.g. "x64 Native Tools Command prompt for VS 2022 Current")
2. Run commands:
```
# Configure
cmake -S . -B build -G Ninja

# Build
cmake --build build

# Test
ctest --test-dir build
# or
.\build\PixelClock_Tests.exe
```