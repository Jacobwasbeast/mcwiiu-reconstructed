#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ANYOBJECTHOLDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ANYOBJECTHOLDER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class Holder : public RootObject
{
public:
    explicit Holder(T* pObject = 0)
        : m_pObject(pObject)
    {
    }

    Holder(const Holder<T>& rOther)
        : m_pObject(rOther.m_pObject)
    {
        rOther.m_pObject = 0;
    }

    ~Holder()
    {
        if (m_pObject != 0)
            delete m_pObject;
    }

    T* GetObject() const
    {
        return m_pObject;
    }

    void SetObject(T* pObject)
    {
        if (m_pObject != 0)
            delete m_pObject;

        m_pObject = pObject;
    }

protected:
    mutable T* m_pObject;
};

template <typename T, typename KeyType>
class AnyObjectHolder : public Holder<T>
{
public:
    explicit AnyObjectHolder(T* pObject = 0)
        : Holder<T>(pObject)
    {
    }

    AnyObjectHolder(const AnyObjectHolder<T, KeyType>& rOther)
        : Holder<T>(rOther)
    {
    }

    virtual ~AnyObjectHolder()
    {
    }
};

}
}

#endif
