cmake_minimum_required(VERSION 3.14)
project(malloclab_handout C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(malloclab_handout
        clock.c
        clock.h
        config.h
        fcyc.c
        fcyc.h
        fsecs.c
        fsecs.h
        ftimer.c
        ftimer.h
        mdriver.c
        memlib.c
        memlib.h
        mm.c
        mm.h)
