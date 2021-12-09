#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <cstdio>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
//    int fd = socket(PF_INET, SOCK_STREAM, 0);
    int fd = socket(AF_INET6, SOCK_STREAM, 0);

    int value = 8;

    int optval;
    socklen_t optlen = sizeof(optval);
    if (0 != getsockopt(fd, IPPROTO_IP, IP_TOS, &optval, &optlen)) {
        return -1;
    }
    printf("IP_TOS got: 0x%x\n", optval);

    value = (value << 2) | (optval & 0x3);
    if (0 != setsockopt(fd, IPPROTO_IP, IP_TOS, &value, sizeof(value))) {
        return -1;
    }
    printf("IP_TOS set to 0x%x\n", value);

    // Get ECN from Traffic Class if IPv6 is available
    if (0 == getsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS, &optval, &optlen)) {
        printf("IPV6_TCLASS got: 0x%x\n", optval);
        value |= (optval & 0x3);
        if (0 != setsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS, &value,
                            sizeof(value))) {
            return -1;
        }
        printf("IPV6_TCLASS set to 0x%x\n", value);
    }

    printf("DONE\n");
    return 0;
}
