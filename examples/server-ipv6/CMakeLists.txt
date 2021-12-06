add_executable(server-ipv6
  main.cpp
)

target_link_libraries(server-ipv6
  myproto
  gRPC::grpc++_unsecure
)
