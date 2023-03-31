#!/bin/bash

./server.py 5555 config.json  &
./client.py localhost 5555 x &
./client.py localhost 5555 y &
./client.py localhost 5555 z &
./client.py localhost 5555 v &
./client.py localhost 5555 w &
