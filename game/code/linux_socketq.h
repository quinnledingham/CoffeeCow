#ifndef SDL_SOCKETQ_H
#define SDL_SOCKETQ_H

internal int
timeout(int sock);

internal int
recvPlatform(int sock, char* buffer, int bufferSize, int flags);

internal int
recvfromPlatform(int sock, char* buffer, int bufferSize, int flags, struct addrinfo *info);

internal int
sendPlatform(int sock, char* buffer, int bytesToSend, int flags);

internal int
sendtoPlatform(int sock, char* buffer, int bytesToSend, int flags, struct addrinfo *info);

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

#endif //SDL_SOCKETQ_H
