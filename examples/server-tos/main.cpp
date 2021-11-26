#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using hello::Greeter;
using hello::HelloRequest;
using hello::HelloReply;

class GreeterServiceImpl final : public Greeter::Service {
    grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request,
                          HelloReply* reply) override {
        std::cout << "Server got called: SayHello() with name=" << request->name() << std::endl;
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
    }
};

class CustomServerBuilderOption: public grpc::ServerBuilderOption {
public:
    CustomServerBuilderOption() {}
    ~CustomServerBuilderOption() {}

    // Alter the ChannelArguments used to create the gRPC server.
    // This will be called inside ServerBuilder::BuildAndStart().
    // We have to push any custom channel arguments into args.
    virtual void UpdateArguments(grpc::ChannelArguments* args){
        std::cout << "Set ChannelArgument: GRPC_ARG_IP_TOS" << std::endl;
        args->SetInt(GRPC_ARG_IP_TOS, 0x20); // TOS Priority -> DSCP/PHB Class: cs1
    }

    virtual void UpdatePlugins(std::vector<std::unique_ptr<grpc::ServerBuilderPlugin>> *plugins) {}
};

int main (void) {
    std::string server_address("127.0.0.1:52231");
    GreeterServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);

    grpc::ServerBuilder builder;
    builder.RegisterService(&service);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Add ServerBuilderOption
    std::unique_ptr<CustomServerBuilderOption> customServerBuilderOption(new CustomServerBuilderOption());
    builder.SetOption(std::move(customServerBuilderOption));

    // Assemble server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown
    server->Wait();

    return 0;
}
