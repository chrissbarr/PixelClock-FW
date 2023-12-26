include(FetchContent)

FetchContent_Declare(
  Button2
  GIT_REPOSITORY https://github.com/LennartHennigs/Button2
  GIT_TAG        bb5ea4b0bdcff179984d8277553058217513c6c1
  CONFIGURE_COMMAND
)

FetchContent_Populate(Button2)

message("${button2_POPULATED}")
message("${button2_SOURCE_DIR}")

set(button2_sources
/src/Button2.cpp
)

list(TRANSFORM button2_sources PREPEND ${button2_SOURCE_DIR})
message("${button2_sources}")

add_library(Button2)
target_sources(Button2 PRIVATE ${button2_sources})
target_include_directories(Button2 PUBLIC "${button2_SOURCE_DIR}/src")