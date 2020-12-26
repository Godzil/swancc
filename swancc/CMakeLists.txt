cmake_minimum_required(VERSION 3.1)

project(swancc)

# Base lib
set(LIBSRC assign.c codefrag.c debug.c declare.c parser.c express.c exptree.c floatop.c function.c gencode.c genloads.c
        glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c
        table.c type.c)

set(LIBHDR include/swancc.h include/swancc/align.h include/swancc/assign.h include/swancc/byteord.h include/swancc/codefrag.h
        include/swancc/condcode.h include/swancc/const.h include/swancc/debug.h include/swancc/declare.h include/swancc/express.h
        include/swancc/exptree.h include/swancc/floatop.h include/swancc/function.h include/swancc/gencode.h include/swancc/genloads.h
        include/swancc/glogcode.h include/swancc/hardop.h include/swancc/input.h include/swancc/label.h include/swancc/loadexp.h
        include/swancc/longop.h include/swancc/os.h include/swancc/output.h include/swancc/parser.h include/swancc/preproc.h
        include/swancc/preserve.h include/swancc/reg.h include/swancc/sc.h include/swancc/scan.h include/swancc/sizes.h
        include/swancc/softop.h include/swancc/state.h include/swancc/table.h include/swancc/type.h include/swancc/types.h)

add_library(swancclib ${LIBSRC} ${LIBHDR})
target_include_directories(swancclib PUBLIC include)

# swancc-cc1
add_executable(swancc-cc1 swancc-cc1.c)
target_include_directories(swancc-cc1 PUBLIC include)
target_link_libraries(swancc-cc1 swancclib)

# swancc
add_executable(swancc swancc.c)
target_include_directories(swancc PUBLIC include)
