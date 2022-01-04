#ifndef SOCKETQ_H
#define SOCKETQ_H

#define TCP 0
#define UDP 1

#define CLIENT 0
#define SERVER 1

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

#endif //SOCKETQ_H
