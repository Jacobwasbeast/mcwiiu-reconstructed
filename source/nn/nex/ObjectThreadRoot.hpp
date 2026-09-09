#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OBJECTTHREADROOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OBJECTTHREADROOT_HPP

#include <cstdint>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

struct OSThread;

namespace nn
{
namespace nex
{

class ObjectThreadRoot : public RootObject
{
public:
    ObjectThreadRoot(const String& name);
    ObjectThreadRoot(const String& name, std::uint32_t priority);
    virtual ~ObjectThreadRoot();

    static ObjectThreadRoot* GetInstance();
    static std::uint32_t GetCurrentThreadID();

    static void SetThreadAffinity(std::uint32_t affinity);
    static void SetDefaultThreadAffinity(std::uint32_t affinity);
    static void SetDefaultPrio(std::uint32_t priority);
    static std::uint32_t GetDefaultPrio();
    static void SetThreadStackSize(std::uint32_t stackSize);
    static std::uint32_t GetThreadStackSize();
    static void UseAutoStack(bool useAutoStack);
    static bool IsUsingAutoStack();

    bool Launch(std::uint32_t stackSize);
    bool Wait(std::uint32_t timeout);
    void SetPriority(std::uint32_t priority);
    void SetThreadName(const String& name);
    void CheckStack();

protected:
    void ReadyToRun();

    virtual void CallObjectMethod() = 0;

private:
    struct ThreadData : public RootObject
    {
        ThreadData();

        OSThread* m_pThread;
        char* m_pStack;
        bool m_bJoined;
    };

    static int ObjectThreadProc(int argc, void* argv);

    bool LaunchImpl();
    void Attach(std::uint32_t threadId);
    void Detach();
    void WrapExceptionHandling();
    void CallObjectMethodWrapper();

    String m_name;
    ThreadData* m_pThreadData;
    std::uint32_t m_threadId;
    std::uint32_t m_priority;
    std::uint32_t m_stackSize;
    bool m_bLaunched;
    bool m_bStarted;
    bool m_bFinished;
    std::uint32_t m_affinity;
    char m_nameChar8[256];
};

}
}

#endif
