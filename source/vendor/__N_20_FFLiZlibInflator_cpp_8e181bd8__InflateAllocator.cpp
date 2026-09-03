#include "vendor/__N_20_FFLiZlibInflator_cpp_8e181bd8__InflateAllocator.hpp"

namespace __N_20_FFLiZlibInflator_cpp_8e181bd8
{
void InflateAllocator::Allocate(unsigned int size)
{
    Allocate(size, 4);
}

void* InflateAllocator::Allocate(unsigned int size, unsigned int alignment)
{
    if (!m_allocator.CanAllocate(size, alignment))
    {
        return 0;
    }
    return (void*)m_allocator.Allocate(size, alignment);
}

void InflateAllocator::Free(void* buffer)
{
}

InflateAllocator::~InflateAllocator()
{
}
}
