add_executable(server-tos
  main.cpp
)

target_link_libraries(server-tos
  myproto
  gRPC::grpc++_unsecure
)
