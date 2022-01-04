#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "win32_socketq.h"

#include <time.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define TCP 0
#define UDP 1

#define CLIENT 0
#define SERVER 1


internal 
int timeout(int sock)
{
    // Waits for a recieve on sock. if none it returns 0.
    fd_set fds;
    int n;
    struct timeval tv;
    
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    
    n = select(sock, &fds, NULL, NULL, &tv);
    if (n == 0)
    {
        fprintf(stderr, "Timeout.. (UDP)\n");
        return 0;
    }
    else if (n < 0)
    {
        fprintf(stderr, "timeout(): select() call failed!\n");
        return 1;
    }
    
    return 1;
}

#define HEADER_BUFFER_SIZE 10

internal int
recvBuffer(int sock, struct addrinfo *info, int protocol, int type, char* buffer, int bufferSize)
{
    char* cursor = buffer;
    
    int bytesRecd;
    int bytesRecdTotal = 0;
    
    int recvBufferSize = 0;
    
    while(1)
    {
        if (protocol == TCP)
        {
            int yo = sizeof(*buffer);
            printf("Waiting for message (TCP).\n");
            bytesRecd = recv(sock, cursor, bufferSize, 0);
            
            if (bytesRecd < 0)
            {
                fprintf(stderr, "%d\n", errno);
                fprintf(stderr, "recvBuffer(): recv() call failed!\n");
                exit(1);
            }
            
            if (bytesRecdTotal == 0)
            {
                char temp[HEADER_BUFFER_SIZE];
                memcpy(temp, buffer, HEADER_BUFFER_SIZE);
                recvBufferSize = atoi(temp);
                bytesRecd = bytesRecd - HEADER_BUFFER_SIZE;
                
                cursor = cursor + HEADER_BUFFER_SIZE;
                memcpy(buffer, cursor, bytesRecd);
            }
            
            
            bytesRecdTotal = bytesRecdTotal + bytesRecd;
            cursor = cursor + bytesRecd;
            
            if (bytesRecdTotal == recvBufferSize)
            {
                break;
            }
        }
        else if (protocol == UDP)
        {
            if (type == CLIENT)
            {
                timeout(sock);
            }
            
            printf("Waiting for message (UDP).");
            bytesRecd = recvfrom(sock, buffer, bufferSize, 0,
                                 info->ai_addr,
                                 reinterpret_cast<int*>(&info->ai_addrlen));
            if (bytesRecd < 0)
            {
                fprintf(stderr, "recvBuffer(): recvfrom() call failed!\n");
                return 0;
            }
            
            bytesRecdTotal = bytesRecd;
            break;
        }
    }
    
    return bytesRecdTotal;
}

internal int
sendBuffer(int sock, struct addrinfo *info, int protocol, char* buffer, int bufferSize)
{
    char header[HEADER_BUFFER_SIZE];
    sprintf(header, "%d", bufferSize);
    
    char* header_buffer = (char*)malloc(sizeof(char) * (HEADER_BUFFER_SIZE + bufferSize));
    memset(header_buffer, 0, sizeof(char) * (HEADER_BUFFER_SIZE + bufferSize));
    
    char* cursor = header_buffer;
    memcpy(cursor, header, HEADER_BUFFER_SIZE);
    cursor = cursor + HEADER_BUFFER_SIZE;
    memcpy(cursor, buffer, bufferSize);
    
    cursor = header_buffer;
    
    int bytesToSend = HEADER_BUFFER_SIZE + bufferSize;
    int bytesSent;
    
    while (bytesToSend > 0)
    {
        if (protocol == TCP)
        {
            printf("Sending message (TCP).\n");
            bytesSent = send(sock, cursor, bytesToSend, 0);
            if (bytesSent < 0)
            {
                fprintf(stderr, "%d\n", errno);
                fprintf(stderr, "sendBuffer(): send() call failed\n");
                exit(1);
            }
        }
        else if (protocol == UDP)
        {
            printf("Sending message (UDP).");
            bytesSent = sendto(sock, cursor, bytesToSend, 0,
                               info->ai_addr, info->ai_addrlen);
            if (bytesSent < 0)
            {
                fprintf(stderr, "sendBuffer(): sendto() call failed!\n");
                exit(1);
            }
        }
        
        cursor = cursor + bytesSent;
        bytesToSend = bytesToSend - bytesSent;
    }
    
    return bytesSent;
}

internal addrinfo*
getHost(char *ip, char* port, int type)
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    
    addrinfo hints, *server_info;
    server_info = (addrinfo*)malloc(sizeof(addrinfo));
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    
    if (type == TCP)
        hints.ai_socktype = SOCK_STREAM;
    else if (type == UDP)
        hints.ai_socktype = SOCK_DGRAM;
    
    getaddrinfo(ip, port, &hints, &server_info);
    
    return server_info;
}

internal addrinfo*
addressInit(char* port, int type)
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    
    addrinfo hints, *server_info;
    server_info = (addrinfo*)malloc(sizeof(addrinfo));
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    
    if (type == TCP)
        hints.ai_socktype = SOCK_STREAM;
    else if (type = UDP)
        hints.ai_socktype = SOCK_DGRAM;
    
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    getaddrinfo(NULL, port, &hints, &server_info);
    //printf("%d\n", server_info->ai_socktype);
    return server_info;
}

internal int socketq(struct addrinfo server_info)
{
    int sock = socket(server_info.ai_family, 
                      server_info.ai_socktype,
                      server_info.ai_protocol);
    if (sock < 0)
    {
        fprintf(stderr, "socketM(): socket() called failed!\n");
        exit(1);
    }
    
    return sock;
}

internal void
connectq(int sock, struct addrinfo server_info)
{
    if (connect(sock, server_info.ai_addr, server_info.ai_addrlen) == -1)
    {
        fprintf(stderr, "connectM(): connect() call failed!\n");
    }
}

internal void
bindq(int sock, struct addrinfo server_info)
{
    if (bind(sock, server_info.ai_addr, server_info.ai_addrlen) == -1)
    {
        fprintf(stderr, "bindq(): bind() call failed!\n");
    }
}

internal void 
listenq(int sock)
{
    if (listen(sock, 5) == -1)
    {
        fprintf(stderr, "listenq(): listen() call failed!\n");
        exit(1);
    }
}

internal int 
acceptq(int sock, struct addrinfo server_info)
{
    int newsock;
    if ((newsock = accept(sock, server_info.ai_addr,
                          reinterpret_cast<int*>(&server_info.ai_addrlen))) == 1)
    {
        fprintf(stderr, "acceptq(): accept() call failed!\n");
        exit(1);
    }
    return newsock;
}