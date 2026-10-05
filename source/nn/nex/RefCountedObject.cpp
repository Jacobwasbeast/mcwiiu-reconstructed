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
    if (AddAndFetch(&m_refCount, -1) != 0 || m_disposing)
        return;

    m_disposing = true;
    delete this;
}

}
}
