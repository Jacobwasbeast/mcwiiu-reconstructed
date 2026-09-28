#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_REFCOUNTEDOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_REFCOUNTEDOBJECT_HPP

#include <stdint.h>

#include <coreinit/atomic.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class RefCountedObject : public RootObject
{
public:
    RefCountedObject();
    virtual ~RefCountedObject();

    void AddRef()
    {
        OSAddAtomic(&m_refCount, 1);
    }
    void ReleaseRef();

    uint32_t GetRefCount() const
    {
        return static_cast<uint32_t>(m_refCount);
    }

private:
    volatile int32_t m_refCount;
    bool m_disposing;
};

}
}

#endif
