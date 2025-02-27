cmake_minimum_required(VERSION 3.12)
project(Repeated LANGUAGES CXX)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Build proto lib
add_library(myproto
  test.proto
)

target_link_libraries(myproto
  PUBLIC
    protobuf::libprotobuf
    gRPC::grpc
    gRPC::grpc++
)

target_include_directories(myproto
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

protobuf_generate(
  TARGET myproto
  LANGUAGE cpp
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(
  TARGET myproto
  LANGUAGE grpc
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

# Server
add_executable(server
  server.cpp
)
target_link_libraries(server
  myproto
  gRPC::grpc++
)

# Client
add_executable(client
  client.cpp
)
target_link_libraries(client
  myproto
  gRPC::grpc++
)
