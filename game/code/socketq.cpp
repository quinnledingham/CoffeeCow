#include "socketq.h"


#if defined(_WIN32)

#include "win32_socketq.h"
#include "win32_socketq.cpp"

#elif __linux__

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