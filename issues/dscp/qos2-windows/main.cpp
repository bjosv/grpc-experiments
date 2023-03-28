#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <MSWSock.h> /* LPFN_CONNECTEX */
#include <qos2.h>

//#include <stdlib.h>
#include <stdio.h>

// Some build settings
//#define USE_IPV6
#define USE_IPV4_IN_IPV6
#define USE_IPV6_DUALSTACK
#define USE_TCP
#define USE_SETFLOW         // Works if run as administrator, pure IPv6 or IPv4
//#define ADD_FLOW_BEFORE_CONNECT

#ifdef USE_IPV6
    //#define DEFAULT_ORIG_ADDR "::1" // NOT IN USE
    #ifdef USE_IPV4_IN_IPV6
      #define DEFAULT_DEST_ADDR "10.10.10.78"
    #else
      #define DEFAULT_DEST_ADDR "fe80::ccdd:e840:b706:9dec" // Works!
  #endif
#else
  //#define DEFAULT_ORIG_ADDR "0.0.0.0" // NOT IN USE

  //#define DEFAULT_DEST_ADDR "127.0.0.1"    // No DSCP when using loopback
  //#define DEFAULT_DEST_ADDR "10.10.10.194" // No DSCP when same machine
  #define DEFAULT_DEST_ADDR "10.10.10.78"  // Works! Using IPv4 to external IPs
#endif

#define DEFAULT_PORT 5001

int main() {
    int status;

    printf("Initialize Winsock...\n");
    WSADATA wsaData;
    status = WSAStartup(MAKEWORD(2,0), &wsaData);
    if (status != 0) {
        printf("WSAStartup failed with error: %d\n", status);
        return 1;
    }

    printf("Initiate QOSCreateHandle...\n");

    // Initialize the QoS version parameter.
    QOS_VERSION    version;
    version.MajorVersion = 1;
    version.MinorVersion = 0;

    // Get a handle to the QoS subsystem.
    HANDLE         qosHandle = NULL;
    BOOL           qosResult;
    qosResult = QOSCreateHandle(&version, &qosHandle);
    if (qosResult != TRUE) {
        printf("QOSCreateHandle failed with error: %ld\n", WSAGetLastError());
    }

    DWORD dwFlags = WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT;
#ifdef USE_IPV6
    printf("Create IPv6 TCP socket...\n");
    SOCKET sock = WSASocket(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
#else
#ifdef USE_TCP
    printf("Create IPv4 TCP socket...\n");
    SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
#else // USE_TCP
    printf("Create IPv4 UDP socket...\n");
    SOCKET sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
#endif // USE_TCP
#endif
    if (sock == INVALID_SOCKET) {
        printf("WSASocket failed: INVALID_SOCKET\n");
        return 1;
    }

    // PrepareSocket
    {
        // grpc_tcp_set_non_block
        {
            unsigned long param = 1;
            DWORD ret;
            status = WSAIoctl(sock, FIONBIO, &param, sizeof(param), NULL, 0, &ret,
                              NULL, NULL);
            if (status != 0) {
                printf("WSAIoctl(non block) failed with error: %d\n", status);
                return 1;
            }
        }
        // enable_socket_low_latency
        {
            BOOL param = TRUE;
            status = ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                                  reinterpret_cast<char*>(&param), sizeof(param));
            if (status == SOCKET_ERROR) {
                status = WSAGetLastError();
                printf("WSAIoctl(low latency) failed with error: %d\n", status);
                return 1;
            }
        }
#ifdef USE_IPV6
        // set_dualstack
        {
#ifdef USE_IPV6_DUALSTACK
            DWORD param = 0;
#else
            DWORD param = 1;
#endif
            status = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&param,
                                sizeof(param));
            if (status != 0) {
                printf("WSAIoctl(dual stack) failed with error: %d\n", status);
                return 1;
            }
        }
#endif
    }

    // Bind local address
    // ConnectEx requires the socket to be initially bound.
#ifdef USE_IPV6
    printf("Bind IPv6..\n");
    struct sockaddr_in6 ip6addr;
    ZeroMemory(&ip6addr, sizeof(ip6addr));
    ip6addr.sin6_family = AF_INET6;
//    inet_pton(AF_INET6, DEFAULT_ORIG_ADDR, &ip6addr.sin6_addr);
    status = bind(sock, (struct sockaddr*) &ip6addr, sizeof(ip6addr));
#else
    printf("Bind IPv4..\n");
    struct sockaddr_in ip4addr;
    ZeroMemory(&ip4addr, sizeof(ip4addr));
    ip4addr.sin_family = AF_INET;
    //inet_pton(AF_INET, DEFAULT_ORIG_ADDR, &ip4addr.sin_addr);
    status = bind(sock, (struct sockaddr*) &ip4addr, sizeof(ip4addr));
#endif


    // Prepare Connect
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));

    // Grab the function pointer for ConnectEx for the specific socket.
    LPFN_CONNECTEX ConnectEx;
    GUID guid = WSAID_CONNECTEX;
    DWORD ioctl_num_bytes;
    status = WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                      &ConnectEx, sizeof(ConnectEx), &ioctl_num_bytes, NULL, NULL);

    if (status != 0) {
        printf("WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER) failed with error: %d\n", status);
        return 1;
    }

#ifdef USE_IPV6
#ifdef USE_IPV4_IN_IPV6
    printf("Use IPv4 in IPv6\n");
    sockaddr_in addr4;
    ZeroMemory(&addr4, sizeof(addr4));
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, DEFAULT_DEST_ADDR, &addr4.sin_addr);
    unsigned char kV4MappedPrefix[] = {0, 0, 0, 0, 0,    0,
                                       0, 0, 0, 0, 0xff, 0xff};
    sockaddr_in6 addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(DEFAULT_PORT);
    memcpy(&addr.sin6_addr.s6_addr[0], kV4MappedPrefix, 12);
    memcpy(&addr.sin6_addr.s6_addr[12], &addr4.sin_addr, 4);
#else // USE_IPV4_IN_IPV6
    printf("Use IPv6\n");
    sockaddr_in6 addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET6, DEFAULT_DEST_ADDR, &addr.sin6_addr);
#endif // USE_IPV4_IN_IPV6
#else
    printf("Use IPv4\n");
    sockaddr_in addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, DEFAULT_DEST_ADDR, &addr.sin_addr);
#endif

#ifdef ADD_FLOW_BEFORE_CONNECT
    // Add socket to flow.
    printf("Add socket to flow before connect\n");
    QOS_FLOWID qosFlowId = 0; // Flow Id must be 0.
    qosResult = QOSAddSocketToFlow(qosHandle,
                                   sock,
                                   (struct sockaddr*) &addr,
                                   // QOSTrafficTypeBackground, // CS1
                                   // QOSTrafficTypeExcellentEffort, // CS5
                                   QOSTrafficTypeAudioVideo, // CS5
                                   //QOSTrafficTypeVoice, // CS7
                                   // QOSTrafficTypeControl, // CS7
                                   QOS_NON_ADAPTIVE_FLOW,
                                   &qosFlowId);
    if (qosResult != TRUE) {
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);
        printf("QOSAddSocketToFlow failed: %S [%ld]\n", s, WSAGetLastError());
        LocalFree(s);
    }
#endif

    // Connect
    printf("Connect to %s:%d\n", DEFAULT_DEST_ADDR, DEFAULT_PORT);
#ifdef USE_TCP
    BOOL success = ConnectEx(sock, (struct sockaddr*) &addr, sizeof(addr), NULL, 0, NULL, &overlapped);
    if (success) {
        printf("ConnectEx succeeded immediately\n");
    } else if (WSAGetLastError() == ERROR_IO_PENDING) {
        printf("ConnectEx pending\n");

        DWORD numBytes;
        success = GetOverlappedResult((HANDLE)sock, &overlapped, &numBytes, TRUE);
        if (success)
            printf("ConnectEx succeeded\n");
        else
            printf("ConnectEx failed: %d\n", WSAGetLastError());
    } else {
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);
        printf("ConnectEx failed: %S [%ld]\n", s, WSAGetLastError());
        LocalFree(s);
        return 1;
    }
#else
    status = WSAConnect(sock, (struct sockaddr*) &addr, sizeof(addr), NULL, NULL, NULL, NULL);
    if (status != NO_ERROR) {
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);
        printf("WSAConnect failed: %S [%ld]\n", s, WSAGetLastError());
        LocalFree(s);
        return 1;
    }
#endif

    // Get local address and port
    struct sockaddr_storage ss;
    socklen_t sslen = sizeof(ss);
    if (getsockname(sock, (struct sockaddr *)&ss, &sslen) == 0) {
        char str[INET6_ADDRSTRLEN]; // IPv6 is longer
        int local_port = 0;
        if (ss.ss_family == AF_INET) {
            struct sockaddr_in *sin = (struct sockaddr_in *) &ss;
            inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str));
            local_port = ntohs(sin->sin_port);
        } else if (ss.ss_family == AF_INET6) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &ss;
            inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str));
            local_port = ntohs(sin6->sin6_port);
        }
        printf("Local: %s:%d\n", str, local_port);
    }

#ifndef ADD_FLOW_BEFORE_CONNECT
    // Add socket to flow.
    printf("Add socket to flow after connect\n");
    QOS_FLOWID qosFlowId = 0; // Flow Id must be 0.
    qosResult = QOSAddSocketToFlow(qosHandle,
                                   sock,
                                   (struct sockaddr*) &addr,
                                   // QOSTrafficTypeBackground, // CS1
                                   // QOSTrafficTypeExcellentEffort, // CS5
                                   QOSTrafficTypeAudioVideo, // CS5
                                   //QOSTrafficTypeVoice, // CS7
                                   // QOSTrafficTypeControl, // CS7
                                   QOS_NON_ADAPTIVE_FLOW,
                                   &qosFlowId);
    if (qosResult != TRUE) {
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);
        printf("QOSAddSocketToFlow failed: %S [%ld]\n", s, WSAGetLastError());
        LocalFree(s);
    }
#endif

#ifdef USE_SETFLOW
    DWORD dscp = 3;
    printf("QOSSetFlow dscp=%d\n", dscp);
    if (QOSSetFlow(qosHandle, qosFlowId, QOSSetOutgoingDSCPValue, sizeof(dscp), &dscp, 0,NULL) != 0)
    {
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);
        printf("QOSSetFlow failed: %S [%ld]\n", s, WSAGetLastError());
        LocalFree(s);
    }
#endif

    printf("Send data...\n");
    const char *sendbuf = "<this is a test>";
    for (int i = 0; i < 3; i++) {
        status = send(sock, sendbuf, (int)strlen(sendbuf), 0);
        if (status == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
        }
    }


    printf("Close QoS handle...\n");
    if (QOSCloseHandle(qosHandle) != TRUE) {
        printf("QOSCloseHandle failed with error: %ld\n", WSAGetLastError());
    }

    // Cleanup
    closesocket(sock);
    status = WSACleanup();
    if (status != 0) {
        printf("WSACleanup failed with error: %d\n", status);
        return 1;
    }

    return 0;
}
