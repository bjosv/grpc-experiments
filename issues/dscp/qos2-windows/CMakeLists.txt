cmake_minimum_required(VERSION 3.12)
project(Examples LANGUAGES CXX)

add_executable(qos-test
  main.cpp
)

target_link_libraries(qos-test
  wsock32
  ws2_32
  qwave.lib
)
