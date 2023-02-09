cmake_minimum_required(VERSION 3.11)
project(project1)

include(FetchContent)
include(CheckIPOSupported)

set(CMAKE_CXX_STANDARD 20)

# fetch json library
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(client
    client.cpp
    common.h
)

add_executable(server
    server.cpp 
    common.h
)

target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)

check_ipo_supported(RESULT supported OUTPUT error)

if(supported)
  message(STATUS "LTO supported")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "LTO not supported")
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -s")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -s")
