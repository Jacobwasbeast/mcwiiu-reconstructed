#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENT_HPP

#include <cstdint>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class String;

class SystemComponent : public RootObject
{
public:
    enum State
    {
        STATE_CREATED = 1,
        STATE_INITIALIZING = 2,
        STATE_READY = 4,
        STATE_IN_USE = 8,
        STATE_TERMINATING = 16,
        STATE_TERMINATED = 64,
        STATE_INITIALIZATION_FAILED = 128,
        STATE_TERMINATION_FAILED = 256
    };

    explicit SystemComponent(const String& name);
    virtual ~SystemComponent();

    virtual void StateTransition(State state);
    virtual void OnInitialize();
    virtual void OnTerminate();
    virtual bool BeginInitialization();
    virtual bool EndInitialization();
    virtual bool BeginTermination();
    virtual bool EndTermination();
    virtual State TestState() const;
    virtual void DoWork();
    virtual bool ValidTransition(State state) const;
    virtual bool UseIsAllowed() const;

    void SetParent(SystemComponent* parent);
    bool SetState(State state, bool force);
    State Initialize();
    State Terminate();
    void WaitForTerminatedState(unsigned int timeout);
    void WaitForReadyState(unsigned int timeout);
    void SetName(const String& name);
    bool IncrementInUseCount(const wchar_t* reason);
    void DecrementInUseCount(const wchar_t* reason);
    static const wchar_t* GetStateString(State state);
    const wchar_t* GetType() const;
    bool IsAKindOf(const wchar_t* type) const;

protected:
    struct NameValue
    {
        std::uint32_t length;
        std::uint32_t representation;
    };

    NameValue m_name;
    std::uint32_t m_state;
    std::int32_t m_inUseCount;
    SystemComponent* m_parent;
};

}
}

#endif
