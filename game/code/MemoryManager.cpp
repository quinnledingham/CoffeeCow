#include "memorymanager.h"

global_variable MemoryManager manager;

internal void*
PermanentStorageAssign(void* newM, int size)
{
    char *csrc = (char*)manager.NextStorage;
    char *cdest = (char*)newM;
    
    for (int i = 0; i < size; i++)
    {
        csrc[i] = cdest[i];
    }
    
    void *returnV = manager.NextStorage;
    manager.NextStorage += size;
    
    return returnV;
}