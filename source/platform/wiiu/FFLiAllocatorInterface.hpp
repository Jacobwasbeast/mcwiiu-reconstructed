#ifndef PLATFORM_WIIU_FFLIALLOCATORINTERFACE_HPP
#define PLATFORM_WIIU_FFLIALLOCATORINTERFACE_HPP

class FFLiAllocatorInterface
{
public:
    virtual ~FFLiAllocatorInterface();
    virtual void Allocate(unsigned int size) = 0;
    virtual void* Allocate(unsigned int size, unsigned int alignment) = 0;
    virtual void Free(void* buffer) = 0;
};

#endif
