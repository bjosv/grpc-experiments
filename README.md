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

### Regular server and client

```
# IPv4
GRPC_VERBOSITY=debug ./server/server "0.0.0.0:52231"
GRPC_VERBOSITY=debug ./client/client "0.0.0.0:52231"

# IPv6
GRPC_VERBOSITY=debug ./server/server "[::1]:52231"
GRPC_VERBOSITY=debug ./client/client "[::1]:52231"
```

### Server using socket mutator changing TOS value (priority)

```
./server-mutator/server-mutator
./client/client "0.0.0.0:52231" & ./client/client "0.0.0.0:52231"
```

### Server and client with TOS value (priority)

```
# Build and install
https://github.com/Nordix/grpc/tree/add-tos-channelargs

# Enable examples in build:
# uncomment `server-tos` and `client-tos`in examples/CMakeLists.txt

sudo tcpdump -v -n -i lo 'port 52231'
GRPC_VERBOSITY=debug ./server-tos/server-tos "127.0.0.1:52231"
GRPC_VERBOSITY=debug ./client-tos/client-tos "127.0.0.1:52231"
```

#### IPv6

```
# Server serving only IPv6 (::1)
GRPC_VERBOSITY=debug ./server-tos/server-tos "[::1]:52231"
GRPC_VERBOSITY=debug ./client-tos/client-tos "[::1]:52231"
```

```
# Disable IPv6:
sudo sysctl -w net.ipv6.conf.all.disable_ipv6=1
sudo sysctl -w net.ipv6.conf.default.disable_ipv6=1
sudo sysctl -w net.ipv6.conf.lo.disable_ipv6=1

# Re-enable IPv6:
sudo sysctl -w net.ipv6.conf.all.disable_ipv6=0
sudo sysctl -w net.ipv6.conf.default.disable_ipv6=0
sudo sysctl -w net.ipv6.conf.lo.disable_ipv6=0

# Verify that IPv6 is enabled
ifconfig -a | grep inet6

# Check server
sudo tcpdump -v -n -i lo 'port 52231'

netstat -lnt
# `:::52231` means both IPv6 and IPv4

cat /proc/sys/net/ipv6/bindv6only
sudo sysctl -w net.ipv6.bindv6only=1
```

### Verbose log

See https://github.com/grpc/grpc/blob/master/TROUBLESHOOTING.md
