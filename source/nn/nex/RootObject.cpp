#include "nn/nex/RootObject.hpp"

#include "nn/nex/MemoryManager.hpp"

namespace nn
{
namespace nex
{

RootObject::~RootObject()
{
}

void* RootObject::operator new(std::size_t size)
{
    return MemoryManager::Allocate(static_cast<unsigned int>(size));
}

void RootObject::operator delete(void* memory)
{
    if (memory == 0)
        return;

    MemoryManager::Free(memory);
}

void* RootObject::operator new[](std::size_t size)
{
    return MemoryManager::Allocate(static_cast<unsigned int>(size));
}

void RootObject::operator delete[](void* memory)
{
    MemoryManager::Free(memory);
}

}
}
