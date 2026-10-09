#include "nn/nex/MemoryManager.hpp"

#include <cstdlib>

namespace nn
{
namespace nex
{

MemoryManager::Allocator MemoryManager::s_allocator = 0;
MemoryManager::Deallocator MemoryManager::s_deallocator = 0;

MemoryManager::~MemoryManager()
{
}

void* MemoryManager::GenericMalloc(unsigned int size)
{
    if (s_allocator != 0)
        return s_allocator(size);
    return std::malloc(size);
}

void* MemoryManager::Allocate(unsigned int size)
{
    const unsigned int alignedSize = (size + 3u) & 0xfffffffcU;
    unsigned char* allocation = static_cast<unsigned char*>(GenericMalloc(alignedSize + 8u));
    *reinterpret_cast<Deallocator*>(allocation) = s_deallocator;
    return allocation + 8;
}

void MemoryManager::GenericFree(Deallocator deallocator, void* block)
{
    if (deallocator != 0)
    {
        deallocator(block);
        return;
    }
    std::free(block);
}

void MemoryManager::Free(void* block)
{
    if (block == 0)
        return;

    unsigned char* allocation = static_cast<unsigned char*>(block) - 8;
    GenericFree(*reinterpret_cast<Deallocator*>(allocation), allocation);
}

void* MemoryManager::AllocateForPbPool(void* block, Deallocator deallocator, void* context)
{
    *reinterpret_cast<Deallocator*>(block) = deallocator;
    *reinterpret_cast<void**>(static_cast<unsigned char*>(block) + 4) = context;
    return static_cast<unsigned char*>(block) + 8;
}

void MemoryManager::BeginProtection()
{
}

void MemoryManager::EndProtection()
{
}

}
}
