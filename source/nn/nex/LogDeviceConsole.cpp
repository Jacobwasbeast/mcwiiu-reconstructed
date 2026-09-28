#include "nn/nex/LogDeviceConsole.hpp"

#include <stdint.h>
#include <cstdio>

namespace nn
{
namespace nex
{

LogDeviceConsole::LogDeviceConsole(void* pHandle)
    : m_pHandle(pHandle != 0 ? pHandle : reinterpret_cast<void*>(
        static_cast<uintptr_t>(0xC07EEC58u)))
{
}

LogDeviceConsole::~LogDeviceConsole()
{
}

void LogDeviceConsole::Output(const wchar_t* pText)
{
    std::printf("%ls\n", pText);
}

}
}
