#include "socketq.h"


#if defined(_WIN32)

#include "win32_socketq.h"
#include "win32_socketq.cpp"

#elif __linux__

#include "linux_socketq.h"
#include "linux_socketq.cpp"

#endif

internal void
createServer(Server* s, char* port, int protocol)
{
    s->protocol = protocol;
    s->info = addressInit(port, s->protocol);
    s->sock = socketq(*s->info);
    bindq(s->sock, *s->info);
    
    if (s->protocol == TCP)
        listenq(s->sock);
}

internal void
waitForConnection(Server* s)
{
    s->sockClient = acceptq(s->sock, *s->info);
}

internal void
recvq(Server* s, char* buffer, int bufferSize)
{
    if (s->protocol == TCP)
        recvBuffer(s->sockClient, s->info, s->protocol, SERVER, buffer, bufferSize);
    else if (s->protocol == UDP)
        recvBuffer(s->sock, s->info, s->protocol, SERVER, buffer, bufferSize);
}

internal void
sendq(Server* s, char* buffer, int bufferSize)
{
    if (s->protocol == TCP)
        sendBuffer(s->sockClient, s->info, s->protocol, buffer, bufferSize);
    else if (s->protocol == UDP)
        sendBuffer(s->sock, s->info, s->protocol, buffer, bufferSize);
}

internal void
createClient(Client* c, char* ip, char* port, int protocol)
{
    c->protocol = protocol;
    c->info = getHost(ip, port, c->protocol);
    c->sock = socketq(*c->info);
    
    if (c->protocol == TCP)
    {
        connectq(c->sock, *c->info);
    }
}

internal void
recvq(Client* c, char* buffer, int bufferSize)
{
    recvBuffer(c->sock, c->info, c->protocol, CLIENT, buffer, bufferSize);
}

internal void
sendq(Client *c, char* buffer, int bufferSize)
{
    sendBuffer(c->sock, c->info, c->protocol, buffer, bufferSize);
}

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
            bytesRecd = recvPlatform(sock, cursor, bufferSize, 0);
            
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
            bytesRecd = recvfromPlatform(sock, buffer, bufferSize, 0, info);
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
            bytesSent = sendPlatform(sock, cursor, bytesToSend, 0);
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