#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/server_builder_plugin.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hello.grpc.pb.h"
#include "socket_mutator.h"

using hello::Greeter;
using hello::HelloRequest;
using hello::HelloReply;

class CustomServerBuilderOption: public grpc::ServerBuilderOption {
public:
    CustomServerBuilderOption(int tos) : tos(tos) {}
    ~CustomServerBuilderOption() {}

    // Alter the ChannelArguments used to create the gRPC server.
    // This will be called inside ServerBuilder::BuildAndStart().
    // We have to push any custom channel arguments into args.
    virtual void UpdateArguments(grpc::ChannelArguments* args);

    virtual void UpdatePlugins(std::vector<std::unique_ptr<grpc::ServerBuilderPlugin>> *plugins) {}
private:
    int tos;
};

// Create a socket mutator that we can register to the ServerBuilder, see:
// grpc/src/core/lib/iomgr/socket_mutator.h
class CustomSocketMutator: public grpc_socket_mutator {
public:
    CustomSocketMutator(int tos);
    ~CustomSocketMutator() {}
    bool mutate(grpc_fd_usage usage, int fd);
private:
    int tos;
};


bool custom_mutator_mutate_fd(int fd, grpc_socket_mutator* pMutator) {
    CustomSocketMutator* m = (CustomSocketMutator *)pMutator;
    return m->mutate(GRPC_FD_SERVER_LISTENER_USAGE, fd);
}

bool custom_mutator_mutate_fd_2(const grpc_mutate_socket_info* info, grpc_socket_mutator* pMutator) {
    CustomSocketMutator* m = (CustomSocketMutator *)pMutator;
    return m->mutate(info->usage, info->fd);
}

#define GPR_ICMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))
int custom_mutator_compare(grpc_socket_mutator* lhs, grpc_socket_mutator* rhs) {
    return GPR_ICMP(lhs, rhs);
}

void custom_mutator_destroy(grpc_socket_mutator* pMutator) {
    CustomSocketMutator* m = (CustomSocketMutator *)pMutator;
    delete m;
}

grpc_socket_mutator_vtable custom_mutator_vtable = {custom_mutator_mutate_fd, custom_mutator_compare, custom_mutator_destroy, custom_mutator_mutate_fd_2};


void CustomServerBuilderOption::UpdateArguments(grpc::ChannelArguments *pArg) {
    CustomSocketMutator *pSocketMutator = new CustomSocketMutator(tos);
    pArg->SetSocketMutator(pSocketMutator);
}


CustomSocketMutator::CustomSocketMutator(int tos) : tos(tos) {
    // Init the grpc_socket_mutator vtable.
    grpc_socket_mutator_init(this, &custom_mutator_vtable);
}

// Mutate socket
bool CustomSocketMutator::mutate(grpc_fd_usage usage, int fd) {
    // Get port
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t addr_len = sizeof(addr);
    getsockname(fd, (struct sockaddr *) &addr, &addr_len);

    std::cout << "Run CustomSocketMutator::mutate:Socketopt() fd=" << fd << " port:" << ntohs(addr.sin_port) << " usage: ";

    switch(usage) {
        case GRPC_FD_CLIENT_CONNECTION_USAGE:
            std::cout << "GRPC_FD_CLIENT_CONNECTION_USAGE";
            break;
        case GRPC_FD_SERVER_LISTENER_USAGE:
            // A client has connected to this server
            std::cout << "GRPC_FD_SERVER_LISTENER_USAGE";
            break;
        case GRPC_FD_SERVER_CONNECTION_USAGE:
            // A server socket has been created
            std::cout << "GRPC_FD_SERVER_CONNECTION_USAGE";
            break;
        default:
            std::cout << "<unknown>";
    }
    std::cout << std::endl;

    int tos_read = 0;
    socklen_t len = sizeof(tos_read);
    if (getsockopt(fd, IPPROTO_IP, IP_TOS, &tos_read, &len) != 0) {
        std::cout << "CustomSocketMutator::mutate:Socketopt(get) failed !" << std::endl;
    }
    std::cout << "Read IP_TOS before change: " << tos_read << std::endl;

    if (setsockopt(fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) != 0) {
        std::cout << "CustomSocketMutator::mutate:Socketopt(set) failed !" << std::endl;
        return 0;
    }

    if (getsockopt(fd, IPPROTO_IP, IP_TOS, &tos_read, &len) != 0) {
        std::cout << "CustomSocketMutator::mutate:Socketopt(get) failed !" << std::endl;
    }
    std::cout << "Read IP_TOS after change: " << tos_read << std::endl;

    return 1;
}


class GreeterServiceImpl final : public Greeter::Service {
    grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request,
                          HelloReply* reply) override {
        std::cout << "Server got called: SayHello() with name=" << request->name() << std::endl;
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
    }
};

int main (void) {
    grpc::EnableDefaultHealthCheckService(true);

    // Server 1
    std::string server_address("127.0.0.1:52231");
    GreeterServiceImpl service;

    grpc::ServerBuilder builder;
    builder.RegisterService(&service);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Add ServerBuilderOption
    std::unique_ptr<CustomServerBuilderOption> customServerBuilderOption(new CustomServerBuilderOption(0x28)); // AF11
    builder.SetOption(std::move(customServerBuilderOption));

    // Assemble server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;


    // Server 2
    std::string server_address2("127.0.0.1:52232");
    GreeterServiceImpl service2;

    grpc::ServerBuilder builder2;
    builder2.RegisterService(&service2);
    builder2.AddListeningPort(server_address2, grpc::InsecureServerCredentials());

    // Add ServerBuilderOption
    std::unique_ptr<CustomServerBuilderOption> customServerBuilderOption2(new CustomServerBuilderOption(0x78)); // AF33
    builder2.SetOption(std::move(customServerBuilderOption2));

    // Assemble server
    std::unique_ptr<grpc::Server> server2(builder2.BuildAndStart());
    std::cout << "Server listening on " << server_address2 << std::endl;

    // Wait for the servers to shutdown
    server->Wait();
    server2->Wait();

    return 0;
}
