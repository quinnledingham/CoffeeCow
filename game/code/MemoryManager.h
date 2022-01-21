#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

struct MemoryManager
{
    char* NextStorage;
};

internal void*
PermanentStorageAssign(void* newM, int size);

#endif //_MEMORY_MANAGER_H
