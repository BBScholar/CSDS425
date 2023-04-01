# Project 2: RDT Implementation

This is my implementation of the RDT sockets. It should handle dropped, corrupted, reordered, and duplicate packets.
Sometimes it freezes part way through, not sure why, but works 95% of the time.
My implementation was tested with a large file (sherlock.txt) as well, it works. The file is around
6.5MB and takes around 5 minutes to send, but works.

### Environment

I used `python 3.11.2` on Fedora Linux 37 (kernel version 6.2.8). Make sure you have 
ZLib installed (it should be on most/all linux machines). ZLib  is used
to calculate the crc32 for all packets.

### How to run.

To run receiver:

```bash
chmod +x receiver.py 
./receiver.py <port> <window_size> <download_file>
#example
./receiver.py 9090 12 download.txt
```

To run sender:
```bash
chmod +x sender.py
./sender.py <server addr> <server port> <window_size> <upload file>
# example 
./sender localhost 9090 12 alice.txt
```
