cmake_minimum_required(VERSION 3.17)
project(CourseraRedBelt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_RELEASE "-O2" )
set(CMAKE_CXX_FLAGS_DEBUG  "-pthread -g")
add_executable(CourseraRedBelt main.cpp search_server.cpp parse.cpp)