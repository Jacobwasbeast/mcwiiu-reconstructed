#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_METHODCALLJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_METHODCALLJOB_HPP

#include "nn/nex/Job.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

class DebugString;

template <class Owner, class Arg, class Base>
class MethodCallJob : public Base
{
public:
    typedef void (Owner::*Method)(Arg);

    MethodCallJob(
        Job::JobType jobType, const DebugString& rDebugString, Method pMethod, Owner* pOwner)
        : Base(jobType, rDebugString)
        , m_pMethod(pMethod)
        , m_pOwner(pOwner)
        , m_argument(Arg())
    {
    }

    virtual ~MethodCallJob()
    {
    }

    virtual void Execute()
    {
        (m_pOwner->*m_pMethod)(m_argument);
    }

    virtual String GetTraceInfo() const
    {
        StringStream stream;

        {
            const String baseTraceInfo = Base::GetTraceInfo();
            stream << static_cast<const wchar_t*>(baseTraceInfo);
        }

        stream << L"\tArgument: ";
        stream.StreamNumber(m_argument);

        return String(stream.GetBuffer());
    }

private:
    Method m_pMethod;
    Owner* m_pOwner;
    Arg m_argument;
};

}
}

#endif
