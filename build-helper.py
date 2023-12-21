Import("env")
print("Script running now!")
env.Append(CXXFLAGS=["-Wno-register", "-Wno-reorder", "-Wno-deprecated-declarations"])


def exclude_from_build(env, node):
    return None

# Remove the FMTlib module sourcefile, as it requires C++20
env.AddBuildMiddleware(exclude_from_build, "*fmt/src/fmt.cc")