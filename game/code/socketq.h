#ifndef SOCKETQ_H
#define SOCKETQ_H

#define TCP 0
#define UDP 1

#define CLIENT 0
#define SERVER 1

#define HEADER_BUFFER_SIZE 10

// Server struct and methods
struct Server
{
    //internal void createServer(Server* s, char* port, int protocol);
    
    struct addrinfo *info;
    int sock;
    int sockClient;
    int protocol;
};

internal void
createServer(Server* s, char* port, int protocol);

internal void
waitForConnection(Server* s);

internal void
recvq(Server* s, char* buffer, int bufferSize);

internal void
sendq(Server* s, char* buffer, int bufferSize);

// Client struct and methods
struct Client
{
    struct addrinfo *info;
    int sock;
    int protocol;
};

internal void
createClient(Client* c, char* ip, char* port, int protocol);

internal void
recvq(Client* c, char* buffer, int bufferSize);

internal void
sendq(Client *c, char* buffer, int bufferSize);

// Generic methods
internal int
recvBuffer(int sock, struct addrinfo *info, int protocol, int type, char* buffer, int bufferSize);

internal int
sendBuffer(int sock, struct addrinfo *info, int protocol, char* buffer, int bufferSize);

#endif //SOCKETQ_H
