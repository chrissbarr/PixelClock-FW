add_library(FastLEDMath STATIC src/flm_hsv2rgb.cpp src/flm_lib8tion.cpp)
# Library is marked as SYSTEM to allow lower warning level. Should review this in future.
target_include_directories(FastLEDMath SYSTEM PUBLIC include/)