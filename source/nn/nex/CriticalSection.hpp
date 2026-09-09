#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CRITICALSECTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CRITICALSECTION_HPP

#include "nn/nex/LockChecker.hpp"
#include "nn/nex/MutexPrimitive.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

extern bool g_criticalSectionDisabled;

class CriticalSection : public RootObject
{
public:
    explicit CriticalSection(unsigned int traceMask);
    CriticalSection(const CriticalSection& other);
    virtual ~CriticalSection();

    CriticalSection& operator=(const CriticalSection& other);

    virtual void Enter();
    virtual void Leave();
    virtual bool IsForce() const;

    void ValidateOwnership(bool owned) const;
    bool HasOwnership() const;
    bool HasNotOwnership() const;

protected:
    MutexPrimitive m_mutex;
    LockChecker m_lockChecker;
};

class ForcedCriticalSection : public CriticalSection
{
public:
    explicit ForcedCriticalSection(unsigned int traceMask)
        : CriticalSection(traceMask)
    {
    }

    virtual ~ForcedCriticalSection();

    virtual void Enter();
    virtual void Leave();
    virtual bool IsForce() const;
};

}
}

#endif
