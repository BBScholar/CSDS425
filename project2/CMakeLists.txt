cmake_minimum_required(VERSION 3.11)
project(project2)

include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_executable(sender
    src/sender.cpp
    src/util.cpp
    src/rdt.cpp
)

add_executable(receiver
    src/receiver.cpp
    src/util.cpp
    src/rdt.cpp
)

check_ipo_supported(RESULT supported OUTPUT error)

if(supported)
  message(STATUS "LTO supported")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "LTO not supported")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -s")
