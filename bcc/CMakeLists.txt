cmake_minimum_required(VERSION 3.1)

project("bcc")

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-write-strings -Wpedantic -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-write-strings -Wpedantic -Werror")

# Base lib
set(LIBSRC codefrag.c debug.c declare.c express.c exptree.c floatop.c function.c gencode.c genloads.c 
        glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c 
        softop.c state.c table.c type.c assign.c)
add_library(bcclib ${LIBSRC})
target_include_directories(bcclib PUBLIC include)
target_include_directories(bcclib PUBLIC include)

# bcc-cc&
add_executable(bcc-cc1 bcc-cc1.c)
target_include_directories(bcc-cc1 PUBLIC include)
target_link_libraries(bcc-cc1 bcclib)

# bcc
add_executable(bcc bcc.c)
target_include_directories(bcc PUBLIC include)
target_link_libraries(bcc bcclib)
