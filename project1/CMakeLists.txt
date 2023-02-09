cmake_minimum_required(VERSION 3.10)
project(project1)

include(FetchContent)
include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE include_files
    
)

add_executable(client
    client.cpp
    common.h
)

add_executable(server
    server.cpp 
    common.h
)

check_ipo_supported(RESULT supported OUTPUT error)

if(supported)
  message(STATUS "LTO supported")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "LTO not supported")
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -s")
