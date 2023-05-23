cmake_minimum_required(VERSION 3.12)
project(Examples LANGUAGES CXX)

# Options
option(ENABLE_DSCP_TESTS "Build DSCP tests" ON)

if(NOT WIN32)
  # GRPC and Protocol Buffers libraries location
  list(APPEND CMAKE_PREFIX_PATH "../install")
endif()

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(protos)
add_subdirectory(server)
add_subdirectory(client)

if(ENABLE_DSCP_TESTS)
  add_subdirectory(client-dscp)
  add_subdirectory(server-dscp)
endif()

if(APPLE)
  add_subdirectory(osx-test)
endif()

if(NOT WIN32)
  add_subdirectory(server-mutator)
  add_subdirectory(client-mutator)
endif()
