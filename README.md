# grpc experiments

## Install
MY_INSTALL_DIR=`pwd`/install

git clone --recurse-submodules -b v1.41.0 https://github.com/grpc/grpc
mkdir -p grpc/build && cd grpc/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ..
make -j 4 all install

## Build examples

mkdir examples/build && cd examples/build
cmake ..

./server/server
