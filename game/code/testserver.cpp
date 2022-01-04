#define internal static 
#define local_persist static 
#define global_variable static

#include "socketq.h"
#include "socketq.cpp"

#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv) 
{
    Server s;
    createServer(&s, argv[1], TCP);
    
    waitForConnection(&s);
    while(1)
    {
        char buffer[500];
        memset(buffer, 0, sizeof(buffer));
        recvq(&s, buffer, 500);
        printf("%s\n", buffer);
        sendq(&s, buffer, 500);
    }
}