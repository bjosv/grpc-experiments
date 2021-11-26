add_executable(client-tos
  main.cpp
)

target_link_libraries(client-tos
  myproto
  gRPC::grpc++_unsecure
)
