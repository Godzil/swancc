cmake_minimum_required(VERSION 3.1)

project(bcc)

# Base lib
set(LIBSRC assign.c codefrag.c debug.c declare.c parser.c express.c exptree.c floatop.c function.c gencode.c genloads.c
        glogcode.c hardop.c input.c label.c loadexp.c longop.c output.c preproc.c preserve.c scan.c softop.c state.c
        table.c type.c sizes.c)

set(LIBHDR include/bcc.h include/bcc/align.h include/bcc/assign.h include/bcc/byteord.h include/bcc/codefrag.h
        include/bcc/condcode.h include/bcc/const.h include/bcc/debug.h include/bcc/declare.h include/bcc/express.h
        include/bcc/exptree.h include/bcc/floatop.h include/bcc/function.h include/bcc/gencode.h include/bcc/genloads.h
        include/bcc/glogcode.h include/bcc/hardop.h include/bcc/input.h include/bcc/label.h include/bcc/loadexp.h
        include/bcc/longop.h include/bcc/os.h include/bcc/output.h include/bcc/parser.h include/bcc/preproc.h
        include/bcc/preserve.h include/bcc/reg.h include/bcc/sc.h include/bcc/scan.h include/bcc/sizes.h
        include/bcc/softop.h include/bcc/state.h include/bcc/table.h include/bcc/type.h include/bcc/types.h)

add_library(bcclib ${LIBSRC} ${LIBHDR})
target_include_directories(bcclib PUBLIC include)

# bcc-cc1
add_executable(bcc-cc1 bcc-cc1.c)
target_include_directories(bcc-cc1 PUBLIC include)
target_link_libraries(bcc-cc1 bcclib)

# bcc
add_executable(bcc bcc.c)
target_include_directories(bcc PUBLIC include)
