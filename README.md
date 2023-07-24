# grpc experiments

This is for Linux, see alternative information for [Windows](windows.md)

## Install

```
MY_INSTALL_DIR=`pwd`/install

git clone --recurse-submodules -b v1.41.0 https://github.com/grpc/grpc
mkdir -p grpc/build && cd grpc/build
cmake -GNinja \
      -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=ON \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
ninja install

(make -j 4 all install)
```

## Build examples

```
mkdir examples/build && cd examples/build
cmake -DCMAKE_BUILD_TYPE=DEBUG ..
make

# To build examples using the channel argument DSCP:
cmake -DCMAKE_BUILD_TYPE=DEBUG -DENABLE_DSCP_TESTS=ON ..
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

### Server using socket mutator changing DSCP value (priority)

```
./server-mutator/server-mutator
./client/client "0.0.0.0:52231" & ./client/client "0.0.0.0:52231"
```

### Server and client with DSCP value (priority)

```
# Build and install (see above)
git clone --recurse-submodules -b add-dscp git@github.com:Nordix/grpc.git

# Build the DSCP examples by enabling them, see `ENABLE_DSCP_TESTS` above.

sudo tcpdump -v -n -i lo 'port 52231'
GRPC_VERBOSITY=debug ./server-dscp/server-dscp "127.0.0.1:52231"
GRPC_VERBOSITY=debug ./client-dscp/client-dscp "127.0.0.1:52231"
```

### Client using socket mutator changing DSCP value (priority)

```
./server/server 127.0.0.1:52231 & ./server/server 127.0.0.1:52232
./client-mutator/client-mutator
sudo tcpdump -i lo -v ip
```

#### IPv6

```
# Server serving only IPv6 (::1)
GRPC_VERBOSITY=debug ./server-dscp/server-dscp "[::1]:52231"
GRPC_VERBOSITY=debug ./client-dscp/client-dscp "[::1]:52231"
```

```
# Get current status regarding IPv6
sudo sysctl net.ipv6.conf.all.disable_ipv6
sudo sysctl net.ipv6.conf.default.disable_ipv6
sudo sysctl net.ipv6.conf.lo.disable_ipv6

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


### GRPC development

```
# Requirements
sudo apt install python3 python3-pip
pip install markupsafe

# Re-generate build files
tools/buildgen/generate_projects.sh

# Check all
tools/distrib/sanitize.sh

# Code format
tools/distrib/clang_format_code.sh

# Build with Bazel and GCC
CC=gcc CXX=g++ bazel build //test/...
```
