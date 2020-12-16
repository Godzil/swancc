cmake_minimum_required(VERSION 3.1)

project("swancc")

# External cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake ${CMAKE_MODULE_PATH})

include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

message("-- Building version ${VERSION}")

# Global define
add_definitions(-DVERSION="${VERSION}")

add_subdirectory(bcc)
