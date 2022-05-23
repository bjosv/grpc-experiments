#include <grpcpp/grpcpp.h>
#include <netinet/in.h>

#include "hello.grpc.pb.h"
#include "socket_mutator.h"

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

//---------------------------------------------------
// Create a socket mutator that we can register, see:
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

#define GPR_ICMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))
int custom_mutator_compare(grpc_socket_mutator* lhs, grpc_socket_mutator* rhs) {
    return GPR_ICMP(lhs, rhs);
}

void custom_mutator_destroy(grpc_socket_mutator* pMutator) {
    CustomSocketMutator* m = (CustomSocketMutator *)pMutator;
    delete m;
}

bool custom_mutator_mutate_fd_2(const grpc_mutate_socket_info* info, grpc_socket_mutator* pMutator) {
    CustomSocketMutator* m = (CustomSocketMutator *)pMutator;
    return m->mutate(info->usage, info->fd);
}

grpc_socket_mutator_vtable custom_mutator_vtable = {custom_mutator_mutate_fd, custom_mutator_compare, custom_mutator_destroy, custom_mutator_mutate_fd_2};

CustomSocketMutator::CustomSocketMutator(int tos) : tos(tos) {
    // Init the grpc_socket_mutator vtable.
    grpc_socket_mutator_init(this, &custom_mutator_vtable);
}

// Mutate socket
bool CustomSocketMutator::mutate(grpc_fd_usage usage, int fd) {
    std::cout << "Run CustomSocketMutator::mutate:Socketopt() fd=" << fd << " usage: ";

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

//---------------------------------------------------

int main(int, char**) {
    // Channel 1
    grpc::ChannelArguments args1;
    args1.SetSocketMutator(new CustomSocketMutator(0x28)); // AF11
    GreeterClient greeter1(grpc::CreateCustomChannel("0.0.0.0:52231",
                                                     grpc::InsecureChannelCredentials(),
                                                     args1));

    // Channel 2: new ip
    grpc::ChannelArguments args2;
    args2.SetSocketMutator(new CustomSocketMutator(0x78)); // AF33
    GreeterClient greeter2(grpc::CreateCustomChannel("0.0.0.0:52232",
                                                     grpc::InsecureChannelCredentials(),
                                                     args2));

    // Channel 3: same ip, but new connection
    grpc::ChannelArguments args3;
    args3.SetSocketMutator(new CustomSocketMutator(0x78)); // AF11
    GreeterClient greeter3(grpc::CreateCustomChannel("0.0.0.0:52231",
                                                     grpc::InsecureChannelCredentials(),
                                                     args3));

    std::cout << "Greeter 1 received: " << greeter1.Hello("user1") << std::endl;
    std::cout << "Greeter 2 received: " << greeter2.Hello("user2") << std::endl;
    std::cout << "Greeter 3 received: " << greeter3.Hello("user3") << std::endl;
    return 0;
}
