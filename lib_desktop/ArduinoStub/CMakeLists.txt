set(sources
src/Print.cpp
src/WString.cpp
#src/stdlib_noniso.c
)

add_library(ArduinoStub "${sources}")
target_include_directories(ArduinoStub PUBLIC include)