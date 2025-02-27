#include <grpcpp/grpcpp.h>

#include "test.grpc.pb.h"

using test::TestService;
using test::TestRequest;
using test::TestReply;

class TestServiceImpl final : public TestService::Service {
    grpc::Status HandleRequest(grpc::ServerContext* context, const TestRequest* request,
                               TestReply* reply) override {

        std::cout << "Server got called: size=" << request->test_id_size() << std::endl;
        for (int i = 0; i < request->test_id_size(); ++i) {
            std::cout << i << "  " << request->test_id(i) << std::endl;
        }

        reply->set_message("OK");
        return grpc::Status::OK;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server address>" << std::endl;
        return 1;
    }
    std::string server_address = argv[1];

    TestServiceImpl service;
    grpc::EnableDefaultHealthCheckService(true);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    // Assemble server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown
    server->Wait();

    return 0;
}
