#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SINGLETHREADCALLPOLICY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SINGLETHREADCALLPOLICY_HPP

#include <cstdint>

#include "nn/nex/CallPolicy.hpp"

namespace nn
{
namespace nex
{

class SingleThreadCallPolicy : public CallPolicy
{
public:
    SingleThreadCallPolicy();
    virtual ~SingleThreadCallPolicy();

    void AcceptOneThread(std::uint32_t threadId);
    void AcceptAnyThread();
    void SetMaximumRecursionLevel(std::uint32_t maximumRecursionLevel);
    bool CurrentThreadIsDispatchingJobs() const;

    virtual void BeginCallImpl();
    virtual void EndCallImpl();

private:
    std::uint32_t m_callDepth;
    std::uint32_t m_threadId;
    std::uint32_t m_maximumRecursionLevel;
    std::uint32_t m_recursionLevel;
};

}
}

#endif
