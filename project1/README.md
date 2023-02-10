# Project 1: Network Chat Application

This is my implementation of the network chat application assigned in programming project 1. 

### Description and Features 


```json 
{"type":"GREETING"}
{"type":"MESSAGE", "message": "Message text."}
{"type": "INCOMING", "message": "Message text.", "origin": "127.0.0.1:9999"}
```

### My System

| Property | Value |
|--|--|
| Language | C++20 |
| Compiler | GCC via Cmake |
| GCC Version | 12.2.1 20221121 |
| System Architecture | X86_64 |
| Operating System | Fedora Linux 37 |
| Kernel Version | v6.1.9-200 |
| CMake Version | 3.25.2 |

As long as your compiler supports c++20, you have CMake version 3.11 or higher, and your kernel isn't ancient, this program should compiler
fine on your Linux machine.

I did not have to run anything as `sudo` on my machine.

### Dependencies:

The nlohmann json library is utilized within this project.

[Github Link](https://github.com/nlohmann/json)

This project dependency is downloaded and compiler completely within Cmake, so no need to install this yourself.

### How to build:

Make sure `cmake` is installed on your system. Ensure that `cmake` is version 3.11 or later, as this is required
in order to properly compile the program.

```bash
mkdir build
cd build 
cmake ..
make
```

### How to run the server application:
From within the build directory
```bash
./server <port>

# for example, to put the server up on port 9090
./server 9090 
```

### How to run the client application:
From within the build directory:
```bash 
./client <host> <port>

# for example, to connect client to localhost:9090 
./client localhost 9090
```

The 'host' parameter can take both IP addresses and hostnames. The reliability to
resolve hostnames besides localhost has not been tested, however.


