#pragma once

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <basetsd.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SSIZE_T ssize_t;
typedef int socklen_t;
typedef SOCKET fd_type;
typedef char socket_opt_type;
typedef int socket_length_type;

#define close closesocket
#define WINDOWS_INT_CAST (int)

#else
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

typedef int fd_type;
typedef int socket_opt_type;
typedef int socket_length_type;
#define WINDOWS_INT_CAST
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define LISTEN_BACKLOG 10

typedef struct
{
    fd_type server_fd;
    struct sockaddr_storage storage;
    socklen_t socklen;
    fd_type client_fd;
} ServerSocket;

ServerSocket initialize_socket(const char* const port)
{
    ServerSocket sock;
    socket_opt_type one = 1;

    struct addrinfo hints = {0, 0, 0, 0, 0, 0, 0, 0};
    struct addrinfo* res = NULL;
    fd_type result = 0;

    sock.socklen = sizeof(sock.storage);

#if defined(_MSC_VER)
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        fprintf(stderr, "Versiion 2.2 of Winsock is not available.\n");
        WSACleanup();
        exit(2);
    }
#endif

    hints.ai_family = AF_UNSPEC; /* use IPv4 or IPv6, whichever */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* fill in my IP for me */

    result = getaddrinfo(NULL, port, &hints, &res);
    if (result == -1)
    {
        puts("getaddrinfo returned failure.");
        abort();
    }

    printf("Binding to %s\n", port);

    sock.server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock.server_fd == -1)
    {
        puts("socket failed");
        abort();
    }

    setsockopt(sock.server_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    result = bind(sock.server_fd, res->ai_addr, WINDOWS_INT_CAST res->ai_addrlen);
    if (result == -1)
    {
        puts("bind failed.");
        abort();
    }

    result = listen(sock.server_fd, LISTEN_BACKLOG);
    if (result == -1)
    {
        puts("listen failed.");
        abort();
    }

    return sock;
}

void server_accept(ServerSocket* sock)
{
    sock->client_fd = accept(sock->server_fd, (struct sockaddr*)&sock->storage, &sock->socklen);
    if (sock->client_fd == -1)
    {
        puts("accept failed");
        abort();
    }
}

size_t server_recv(ServerSocket* sock, unsigned char* buf, size_t n)
{
    ssize_t bytes = recv(sock->client_fd, (char*)buf, WINDOWS_INT_CAST n, 0);
    if (bytes < 0)
    {
        puts("recv failed");
        abort();
    }
    return (size_t)bytes;
}

void server_send(ServerSocket* sock, unsigned char* buf, size_t n)
{
    send(sock->client_fd, (char*)buf, WINDOWS_INT_CAST n, 0);
}

void free_socket(ServerSocket* sock)
{
    close(sock->server_fd);
    close(sock->client_fd);
}
