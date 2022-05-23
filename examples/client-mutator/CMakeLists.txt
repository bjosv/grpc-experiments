add_executable(client-mutator
  main.cpp
)

target_link_libraries(client-mutator
  myproto
  gRPC::grpc++_unsecure
)
