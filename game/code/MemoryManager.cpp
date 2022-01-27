#include "memorymanager.h"

global_variable MemoryManager manager;

internal void
MemoryCopy(void* Dest, void* Source, int Size)
{
    char* CharDest = (char*)Dest;
    char* CharSource = (char*)Source;
    for (int i = 0; i < Size; i++)
    {
        CharDest[i] = CharSource[i];
    }
}

internal void*
PermanentStorageBlank(int size)
{
    void *returnV = manager.NextStorage;
    manager.NextStorage += size;
    
    return returnV;
}

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

internal void*
PermanentStorageAssign(const unsigned char* newM, int size)
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