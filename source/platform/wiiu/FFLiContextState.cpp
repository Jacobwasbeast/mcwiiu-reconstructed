#include "platform/wiiu/FFLiContextState.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"

void GX2SetupContextStateEx(_GX2ContextState* state, int profilingEnabled);
void FFLiTempSetContextState(const _GX2ContextState* contextState);

FFLiContextState::FFLiContextState()
    : m_isSetup(false), m_contextState(0)
{
}

FFLiContextState::~FFLiContextState()
{
}

unsigned int FFLiContextState::GetBufferSize()
{
    return 0xa200;
}

void FFLiContextState::Initialize(FFLiBufferAllocator* allocator)
{
    m_contextState = (_GX2ContextState*)allocator->Allocate(0xa100, 0x100);
}

void FFLiContextState::Setup()
{
    m_isSetup = true;
    GX2SetupContextStateEx(m_contextState, 1);
}

void FFLiContextState::SetupEx(bool profilingEnabled)
{
    m_isSetup = true;
    GX2SetupContextStateEx(m_contextState, profilingEnabled ? 1 : 0);
}

void FFLiContextState::Bind(bool setupIfNeeded)
{
    if (setupIfNeeded && !m_isSetup)
    {
        Setup();
        return;
    }
    FFLiTempSetContextState(m_contextState);
}

void FFLiContextState::SetDefaultState()
{
    FFLiTempSetContextState(0);
}
