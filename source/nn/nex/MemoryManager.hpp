#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MEMORYMANAGER_HPP

#include <cstddef>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class MemoryManager : public RootObject
{
public:
    typedef void* (*Allocator)(unsigned int);
    typedef void (*Deallocator)(void*);

    static void SetCallbacks(Allocator allocator, Deallocator deallocator);

    virtual ~MemoryManager();

    static void* GenericMalloc(unsigned int size);
    static void* Allocate(unsigned int size);
    static void GenericFree(Deallocator deallocator, void* block);
    static void Free(void* block);
    static void* AllocateForPbPool(void* block, Deallocator deallocator, void* context);

    static void ShutdownDefaultMemoryManager();
    void BeginProtection();
    void EndProtection();

private:
    unsigned int m_memoryUsage;
};

}
}

#endif
