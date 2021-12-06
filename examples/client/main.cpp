#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using hello::Greeter;
using hello::HelloRequest;
using hello::HelloReply;

class GreeterClient {
 public:
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(Greeter::NewStub(channel)) {}

    std::string Hello(const std::string& user) {

        HelloRequest request;
        request.set_name(user);

        HelloReply reply;
        ClientContext context;
        Status status = stub_->SayHello(&context, request, &reply);

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

    GreeterClient greeter(
    grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

    std::string user("user");
    std::string reply = greeter.Hello(user);
    std::cout << "Greeter received: " << reply << std::endl;

    return 0;
}
