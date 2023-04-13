# Project 3: Distance-Vector Algorithm


### Usage:

```bash
# start server
./server.py 5555 config.json &

# start all clients
./client.py localhost 5555 u &
./client.py localhost 5555 v &
./client.py localhost 5555 w &
./client.py localhost 5555 x &
./client.py localhost 5555 y &
./client.py localhost 5555 z &

# or (for the configuration mentioned within the assignment only,
# start all clients at once, each in a separate thread
./all_clients.py localhost 5555

# to print results
cat result*
```

Results will be appended to `result_<node name>.txt` in this directory.


### Environment

Operating system: Fedora Linux 37 
Kernel Version: 6.2.9
Programming language: Python 3
Python version: 3.11.2

It's a safe assumption that this program should run on any modern linux distribution.
I am using the standard `socket` and `json` libraries provided in the python3 
standard library.
