# Project 4: HTTP Proxy

### Usage

```bash 
# allow execution if necessary
chmod +x server.py
# to run 
./server.py <port>
# for example
./server.py 8080
```


```bash
# to test
export http_proxy=http://127.0.0.1:8080  # or another port 
curl google.com/
```

Sometimes, `wget` seems to switch over to https, even when using an explicit `http://` in the url.
Curl does not seem to exhibit this behavior

### Environment
Fedora 37 on x86 architecture.\
Linux Kernel version: 6.2.9\
Python version: 3.11.2
Python libraries: all build-in libraries (regex, socket, select)
