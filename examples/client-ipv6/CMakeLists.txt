add_executable(client-ipv6
  main.cpp
)

target_link_libraries(client-ipv6
  myproto
  gRPC::grpc++_unsecure
)
