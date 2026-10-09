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

    static void SetCallbacks(Allocator allocator, Deallocator deallocator)
    {
        s_allocator = allocator;
        s_deallocator = deallocator;
    }

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
    static Allocator s_allocator;
    static Deallocator s_deallocator;

    unsigned int m_memoryUsage;
};

}
}

#endif
