#ifndef GAME_CORE_CRITICALSECTION_HPP
#define GAME_CORE_CRITICALSECTION_HPP

struct OSFastMutex;

struct OSThreadSimpleQueue
{
    void* head;
    void* tail;
};

struct OSFastMutexLink
{
    OSFastMutex* next;
    OSFastMutex* prev;
};

struct OSFastMutex
{
    unsigned int tag;
    const char* name;
    unsigned int owner;
    OSThreadSimpleQueue queue;
    OSFastMutexLink link;
    unsigned int state;
    int count;
    unsigned int contendedCount;
    unsigned int acquiredCount;
};

void InitializeCriticalSection(OSFastMutex* pCriticalSection);
void InitializeCriticalSectionAndSpinCount(OSFastMutex* pCriticalSection, unsigned long spinCount);
void EnterCriticalSection(OSFastMutex* pCriticalSection);
int TryEnterCriticalSection(OSFastMutex* pCriticalSection);
void LeaveCriticalSection(OSFastMutex* pCriticalSection);
void DeleteCriticalSection(OSFastMutex* pCriticalSection);

#endif
