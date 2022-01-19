#define internal static 
#define local_persist static 
#define global_variable static

#include "socketq.h"
#include "socketq.cpp"

#include <stdlib.h>
#include <stdio.h>


internal void
getUserInput(char* prompt, char* buffer)
{
    char c;
    printf("%s", prompt);
    int i = 0;
    while ((c = getchar()) != '\n')
    {
        buffer[i] = c;
        i++;
    }
    
    buffer[i] = '\0';
}

int
main(int argc, char **argv) 
{
    Client c;
    createClient(&c, argv[1], argv[2], TCP);
    
    while(1)
    {
        char userIn[50];
        memset(userIn, 0, sizeof(userIn));
        char* e = "Enter: ";
        getUserInput(e, userIn);
        sendq(&c, userIn, 50);
        
        char buffer[500];
        memset(buffer, 0, sizeof(buffer));
        recvq(&c, buffer, 500);
        printf("%s\n", buffer);
    }
}