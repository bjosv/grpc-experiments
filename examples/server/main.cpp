#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using hello::Greeter;
using hello::HelloRequest;
using hello::HelloReply;

class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext* context, const HelloRequest* request,
                 HelloReply* reply) override {
        std::cout << "Server got called: SayHello() with name=" << request->name() << std::endl;
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return Status::OK;
    }
};

int main (void) {
    string server_address("0.0.0.0:52231");
    GreeterServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Assemble server
    unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown
    server->Wait();

    return 0;
}
