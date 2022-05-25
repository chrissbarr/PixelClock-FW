Import("env")
print("Script running now!")
env.Append(CXXFLAGS=["-Wno-register", "-Wno-reorder", "-Wno-deprecated-declarations"])