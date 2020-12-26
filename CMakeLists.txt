cmake_minimum_required(VERSION 3.1)

project("swancc")

# External cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake ${CMAKE_MODULE_PATH})

include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

message("-- Building version ${VERSION}")

option(WARN_AS_ERR "Compile with -Werror" ON)

set_property(GLOBAL PROPERTY C_STANDARD 99)
set_property(GLOBAL PROPERTY C_EXTENSIONS OFF)

# -Wno-unused-parameter -Wno-unused-result -Wno-write-strings
set(BASE_CFLAGS "-Wall -Wextra -Wold-style-cast -Wold-style-definition -Wpedantic -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wconversion -Wno-sign-conversion -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "${BASE_CFLAGS}")
set(CMAKE_CXX_FLAGS "${BASE_CFLAGS}")

if (WARN_AS_ERR)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

# Global define
add_definitions(-DVERSION="${VERSION}")

add_subdirectory(swancc)
