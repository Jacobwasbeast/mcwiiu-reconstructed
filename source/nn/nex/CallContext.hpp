#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CALLCONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CALLCONTEXT_HPP

#include <cstdint>

#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class CallContext : public RefCountedObject
{
public:
    enum State
    {
        Ready = 0,
        CallInProgress = 1,
        CallSucceeded = 2,
        CallFailed = 3,
        CallCanceled = 4
    };

    CallContext();
    virtual ~CallContext();

    std::uint8_t Wait(std::uint32_t timeout);
    State GetState() const;
    const qResult& GetResult() const;

protected:
    static bool PreventBlockCallCallback(RootObject* object, std::uint32_t value);
    virtual void BeginTransition(State state, const qResult& result, bool publishResult);
    State m_state;
    std::uint32_t m_flags;
    qResult m_result;
};

}
}

#endif
