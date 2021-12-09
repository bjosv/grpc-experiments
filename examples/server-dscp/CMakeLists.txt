add_executable(server-dscp
  main.cpp
)

target_link_libraries(server-dscp
  myproto
  gRPC::grpc++_unsecure
)
