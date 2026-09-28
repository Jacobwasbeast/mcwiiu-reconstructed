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
    if (OSAddAtomic(&m_refCount, -1) != 1 || m_disposing)
        return;

    m_disposing = true;
    delete this;
}

}
}
