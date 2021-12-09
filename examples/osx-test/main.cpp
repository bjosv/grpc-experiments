#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <assert.h>

static void test_dscp(int fd) {
    int value = 8;

    int optval;
    socklen_t optlen = sizeof(optval);
    if (0 != getsockopt(fd, IPPROTO_IP, IP_TOS, &optval, &optlen)) {
        assert(0);
    }
    printf("  IP_TOS got: 0x%x\n", optval);

    value = (value << 2) | (optval & 0x3);
    if (0 != setsockopt(fd, IPPROTO_IP, IP_TOS, &value, sizeof(value))) {
        assert(0);
    }
    printf("  IP_TOS set to 0x%x\n", value);

    if (0 != getsockopt(fd, IPPROTO_IP, IP_TOS, &optval, &optlen)) {
        assert(0);
    }
    printf("  IP_TOS got: 0x%x\n", optval);

    // Get ECN from Traffic Class if IPv6 is available
    if (0 == getsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS, &optval, &optlen)) {
        printf("  IPV6_TCLASS got: 0x%x\n", optval);
        value |= (optval & 0x3);
        if (0 != setsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS, &value,
                            sizeof(value))) {
            assert(0);
        }
        printf("  IPV6_TCLASS set to 0x%x\n", value);

        if (0 != getsockopt(fd, IPPROTO_IPV6, IPV6_TCLASS, &optval, &optlen)) {
            assert(0);
        }
        printf("  IPV6_TCLASS got: 0x%x\n", optval);
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    const int off = 0;
    const int on = 1;

    printf("Testing IPv4\n");
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    test_dscp(fd);
    close(fd);

    printf("Testing IPv6\n");
    fd = socket(AF_INET6, SOCK_STREAM, 0);
    assert(0 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)));
    test_dscp(fd);
    close(fd);


    printf("Testing dualstack\n");
    fd = socket(AF_INET6, SOCK_STREAM, 0);
    assert(0 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off)));
    test_dscp(fd);
    close(fd);

    printf("DONE\n");
    return 0;
}
