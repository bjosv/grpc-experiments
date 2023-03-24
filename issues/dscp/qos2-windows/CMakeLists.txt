cmake_minimum_required(VERSION 3.12)
project(Examples LANGUAGES CXX)

add_executable(qostest
  main.cpp
)

target_link_libraries(qostest
  wsock32
  ws2_32
  qwave.lib
)
