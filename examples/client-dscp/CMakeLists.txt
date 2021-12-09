add_executable(client-dscp
  main.cpp
)

target_link_libraries(client-dscp
  myproto
  gRPC::grpc++_unsecure
)
