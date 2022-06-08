#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <iostream>

#include "qlib/types.h"
#include "qlib/socketq.cpp"

int main(int argc, const char** argv)
{
    Server server = {};
    server.create("44575", TCP);
    server.waitForConnection();
    
    char Buffer[1000];
    server.recvq(Buffer, 1000);
    printf(Buffer);
}