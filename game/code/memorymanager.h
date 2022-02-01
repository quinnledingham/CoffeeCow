#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

struct MemoryManager
{
    char* NextStorage;
};

internal void*
PermanentStorageAssign(void* newM, int size);

internal void
MemoryCopy(void* Dest, void* Source, int Size);

#endif //_MEMORY_MANAGER_H
