#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using namespace std;

using hello::Greeter;
using hello::HelloRequest;
using hello::HelloReply;

class GreeterClient {
 public:
    GreeterClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(Greeter::NewStub(channel)) {}

    std::string Hello(const std::string& user) {

        HelloRequest request;
        request.set_name(user);

        HelloReply reply;
        grpc::ClientContext context;
        grpc::Status status = stub_->SayHello(&context, request, &reply);

        if (status.ok()) {
            return reply.message();
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server address>" << std::endl;
        return 1;
    }
    string address = argv[1];

    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_IP_TOS_TRAFFIC_CLASS, 0x40); // TOS Immediate -> DSCP/PHB Class: cs2

    // Create a custom channel
    GreeterClient greeter(
    grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), args));

    std::string user("user");
    std::string reply = greeter.Hello(user);
    std::cout << "Greeter received: " << reply << std::endl;

    return 0;
}
