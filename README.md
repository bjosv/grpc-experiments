# grpc experiments

## Install

```
MY_INSTALL_DIR=`pwd`/install

git clone --recurse-submodules -b v1.41.0 https://github.com/grpc/grpc
mkdir -p grpc/build && cd grpc/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ..
make -j 4 all install
```

## Build examples

```
mkdir examples/build && cd examples/build
cmake -DCMAKE_BUILD_TYPE=DEBUG ..
```

## Run examples

### Server and client

```
./server/server
./client/client & ./client/client
```

### Server using socket mutator

```
./server-mutator/server-mutator
./client/client & ./client/client
```

### Server and client with TOS value (priority)

```
# Build and install
https://github.com/Nordix/grpc/tree/add-tos-channelargs

# Enable examples in build:
# uncomment `server-tos` and `client-tos`in examples/CMakeLists.txt

sudo tcpdump -v -n -i lo 'port 52231'
./server-tos/server-tos
./client-tos/client-tos
```

### IPv6

```
# Verify that IPv6 is enabled
ifconfig -a | grep inet6

# Server serving both IPv4 and IPv6 (0.0.0.0)
./server/server
./client/client
./client-ipv6/client-ipv6

# Server serving only IPv6 (::1)
./server-ipv6/server-ipv6
./client/client
./client-ipv6/client-ipv6

# Check server
sudo tcpdump -v -n -i lo 'port 52231'

netstat -lnt
# `:::52231` means both IPv6 and IPv4
```
