#ifndef SOURCE_VENDOR___N_20_FFLIZLIBINFLATOR_CPP_8E181BD8__INFLATEALLOCATOR_HPP
#define SOURCE_VENDOR___N_20_FFLIZLIBINFLATOR_CPP_8E181BD8__INFLATEALLOCATOR_HPP

#include "platform/wiiu/FFLiAllocatorInterface.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"

namespace __N_20_FFLiZlibInflator_cpp_8e181bd8
{
class InflateAllocator : public FFLiAllocatorInterface
{
public:
    virtual ~InflateAllocator();
    virtual void Allocate(unsigned int size);
    virtual void* Allocate(unsigned int size, unsigned int alignment);
    virtual void Free(void* buffer);

private:
    FFLiBufferAllocator m_allocator;
};
}

#endif
