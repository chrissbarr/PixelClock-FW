include(FetchContent)

FetchContent_Declare(
  FastLED
  GIT_REPOSITORY https://github.com/chrissbarr/FastLED.git
  GIT_TAG        22fbd859cb44f228ac1b88047f6c44c118532089
  CONFIGURE_COMMAND
)

FetchContent_Populate(FastLED)

message("${fastled_POPULATED}")
message("${fastled_SOURCE_DIR}")

set(fastled_sources
/src/bitswap.cpp
/src/colorpalettes.cpp
/src/colorutils.cpp
/src/FastLED.cpp
/src/hsv2rgb.cpp
/src/lib8tion.cpp
/src/noise.cpp
/src/platforms.cpp
/src/power_mgt.cpp
/src/wiring.cpp
)

list(TRANSFORM fastled_sources PREPEND ${fastled_SOURCE_DIR})
message("${fastled_sources}")

add_library(FastLED)
target_sources(FastLED PRIVATE ${fastled_sources})
target_include_directories(FastLED PUBLIC ${fastled_SOURCE_DIR})
#target_include_directories(Display PUBLIC lib/display/include/)