cmake_minimum_required(VERSION 3.14)
project(source C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(thingspeak)
include_directories(Util)

add_executable(source
        thingspeak/thingspeak.c
        thingspeak/thingspeak.h
        Util/queue.c
        Util/queue.h
        thingspeak/thingspeak_debug.c
        thingspeak/thingspeak_debug.h
        thingspeak/thingspeak_debug_main.c)
