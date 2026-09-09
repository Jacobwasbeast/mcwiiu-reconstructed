#include "nn/nex/Platform.hpp"

#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

namespace nn
{
namespace nex
{

Platform* Platform::s_instance = 0;

Platform::Platform()
{
    s_instance = this;
}

Platform::~Platform()
{
}

void Platform::CreateInstance()
{
    s_instance = new Platform();
}

void Platform::DeleteInstance()
{
    if (s_instance == 0)
        return;

    delete s_instance;
}

void Platform::Sleep(unsigned int milliseconds)
{
    OSSleepTicks(OSMillisecondsToTicks(milliseconds));
}

void Platform::YieldThread()
{
    OSYieldThread();
}

int Platform::Breakpoint()
{
    __builtin_trap();
    return 0;
}

std::int64_t Platform::GetTick()
{
    return OSGetTime();
}

}
}
