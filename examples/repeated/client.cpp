#include <grpcpp/grpcpp.h>

#include "test.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using test::TestService;
using test::TestRequest;
using test::TestReply;

class TestClient {
public:
    TestClient(std::shared_ptr<Channel> channel)
        : stub_(TestService::NewStub(channel)) {}

    std::string HandleRequest() {

        // Create request containing a repeated field
        TestRequest request;
        request.add_test_id(2);
        request.add_test_id(4);

        TestReply reply;
        ClientContext context;
        Status status = stub_->HandleRequest(&context, request, &reply);

        if (status.ok()) {
            return reply.message();
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<TestService::Stub> stub_;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server address>" << std::endl;
        return 1;
    }
    string address = argv[1];

    TestClient client(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

    std::string reply = client.HandleRequest();
    std::cout << "Reply: " << reply << std::endl;

    return 0;
}
