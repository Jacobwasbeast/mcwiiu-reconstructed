#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OBJECTTHREAD_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OBJECTTHREAD_HPP

#include "nn/nex/ObjectThreadRoot.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

template<class TObject, class TParam>
class ObjectThread : public ObjectThreadRoot
{
public:
    typedef void (TObject::*Method)(TParam);

    explicit ObjectThread(const String& name)
        : ObjectThreadRoot(name)
        , m_method(0)
        , m_pObject(0)
    {
    }

    virtual ~ObjectThread()
    {
    }

    void SetObjectMethod(Method method, TObject* pObject, TParam param)
    {
        m_param = param;
        m_pObject = pObject;
        m_method = method;
    }

protected:
    virtual void CallObjectMethod()
    {
        ReadyToRun();
        (m_pObject->*m_method)(m_param);
    }

private:
    Method m_method;
    TObject* m_pObject;
    TParam m_param;
};

}
}

#endif
