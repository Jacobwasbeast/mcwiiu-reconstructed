#include "nn/nex/RefCountedObject.hpp"

namespace nn
{
namespace nex
{

RefCountedObject::RefCountedObject()
    : m_refCount(1)
    , m_disposing(false)
{
}

RefCountedObject::~RefCountedObject()
{
    m_refCount = 0;
}

void RefCountedObject::ReleaseRef()
{
    const std::uint32_t count = __atomic_sub_fetch(&m_refCount, 1u, __ATOMIC_SEQ_CST);
    if (count != 0 || m_disposing)
        return;

    m_disposing = true;
    delete this;
}

}
}
