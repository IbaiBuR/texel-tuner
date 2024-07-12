cmake_minimum_required(VERSION 3.12)
project(tuner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -flto=auto -DNDEBUG -march=native -mtune=native")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

file(GLOB SRCS "*.cpp" "engines/*.cpp")
add_executable(tuner ${SRCS})

target_link_libraries(tuner PRIVATE Threads::Threads)