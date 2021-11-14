add_executable(server-mutator
  main.cpp
)

target_link_libraries(server-mutator
  myproto
  gRPC::grpc++_unsecure
)
