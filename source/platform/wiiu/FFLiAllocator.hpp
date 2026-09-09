#ifndef PLATFORM_WIIU_FFLIALLOCATOR_HPP
#define PLATFORM_WIIU_FFLIALLOCATOR_HPP

#include <coreinit/memheap.h>

class FFLiAllocator
{
public:
    FFLiAllocator();
    ~FFLiAllocator();
    bool IsValid() const;
    void Init(void* buffer, unsigned int size);
    void* Allocate(unsigned int size, unsigned int alignment);
    void Free(void* block);

private:
    MEMHeapHandle mHeap;
};

#endif
