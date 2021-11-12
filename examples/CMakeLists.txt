cmake_minimum_required(VERSION 3.12)
project(Examples LANGUAGES CXX)

# GRPC and Protocol Buffers libraries location
list(APPEND CMAKE_PREFIX_PATH "../install")

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(protos)
add_subdirectory(server)
add_subdirectory(client)
