cmake_minimum_required(VERSION 3.17)
project(HW3_OS)

set(CMAKE_CXX_STANDARD 14)

include_directories(Code_Skeleton)

add_executable(HW3_OS
        Code_Skeleton/Game.cpp
        Code_Skeleton/Game.hpp
        Code_Skeleton/Headers.hpp
        Code_Skeleton/main.cpp
        Code_Skeleton/PCQueue.hpp
        Code_Skeleton/Semaphore.cpp
        Code_Skeleton/Semaphore.hpp
        Code_Skeleton/Thread.hpp
        Code_Skeleton/utils.cpp
        Code_Skeleton/utils.hpp)
