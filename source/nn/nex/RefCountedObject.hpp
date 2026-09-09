#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_REFCOUNTEDOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_REFCOUNTEDOBJECT_HPP

#include <cstdint>

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
        __atomic_add_fetch(&m_refCount, 1u, __ATOMIC_SEQ_CST);
    }
    void ReleaseRef();

    std::uint32_t GetRefCount() const
    {
        return m_refCount;
    }

private:
    std::uint32_t m_refCount;
    bool m_disposing;
};

}
}

#endif
