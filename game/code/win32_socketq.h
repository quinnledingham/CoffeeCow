#ifndef WIN32_SOCKETQ_H
#define WIN32_SOCKETQ_H

internal 
int timeout(int sock);

internal int
recvBuffer(int sock, struct addrinfo *info, int protocol, int type, char* buffer, int bufferSize);

internal int
sendBuffer(int sock, struct addrinfo *info, int protocol, char* buffer, int bufferSize);

internal addrinfo*
getHost(char *ip, char* port, int type);

internal addrinfo*
addressInit(char* port, int type);

internal int socketq(struct addrinfo server_info);

internal void
connectq(int sock, struct addrinfo server_info);

internal void
bindq(int sock, struct addrinfo server_info);

internal void 
listenq(int sock);

internal int 
acceptq(int sock, struct addrinfo server_info);

#endif //WIN32_SOCKETQ_H
