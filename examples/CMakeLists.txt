cmake_minimum_required(VERSION 3.12)
project(Examples LANGUAGES CXX)

# Options
option(ENABLE_DSCP_TESTS "Build DSCP tests" OFF)

# GRPC and Protocol Buffers libraries location
list(APPEND CMAKE_PREFIX_PATH "../install")

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(protos)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(server-mutator)
if(ENABLE_DSCP_TESTS)
  add_subdirectory(client-dscp)
  add_subdirectory(server-dscp)
endif()
