#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <qos2.h>

#include <MSWSock.h> /* LPFN_CONNECTEX */

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 6379

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

    printf("Create socket...\n");
    DWORD dwFlags = WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT;
    SOCKET sock = WSASocket(AF_INET6, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
                            WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
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
        // set_dualstack
        {
            DWORD param = 0;
            status = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&param,
                                sizeof(param));
            if (status != 0) {
                printf("WSAIoctl(dual stack) failed with error: %d\n", status);
                return 1;
            }
        }
    }

    // Bind local address
    // ConnectEx requires the socket to be initially bound.
    struct sockaddr_in6 ip6addr;
    ZeroMemory(&ip6addr, sizeof(ip6addr));
    ip6addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "::1", &ip6addr.sin6_addr);
    status = bind(sock, (struct sockaddr*) &ip6addr, sizeof(ip6addr));

    // Connect
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

    sockaddr_in6 addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET6, "::1", &addr.sin6_addr);

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
        printf("ConnectEx failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Add socket to flow.
    QOS_FLOWID qosFlowId = 0; // Flow Id must be 0.
    qosResult = QOSAddSocketToFlow(qosHandle,
                                   sock,
                                   (struct sockaddr*) &addr,
                                   QOSTrafficTypeExcellentEffort,
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
