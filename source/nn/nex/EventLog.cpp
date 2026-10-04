#include "nn/nex/EventLog.hpp"

#include <stdint.h>
#include <cwchar>

#include "nn/nex/GlobalVariables.hpp"

namespace nn
{
namespace nex
{

EventLog::EventLog() : m_logLevel(Info), m_field14(false)
{
}

EventLog::~EventLog()
{
}

EventLog* EventLog::GetInstance()
{
    return &GlobalVariables::GetInstancePtr()->GetEventLog();
}

const wchar_t* EventLog::LogLevelToString(LogLevel level)
{
    static const wchar_t* const names[] = {L"Debug", L"Info", L"Warning", L"Error", L"Fatal", L"Always"};
    if (level != Unknown && static_cast<unsigned int>(level) <= Always)
        return names[level - Debug];
    return L"Unknown";
}

void EventLog::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void EventLog::AddCustomPrefix(wchar_t* buffer, int capacity, void* context)
{
    const LogLevel level = static_cast<LogLevel>(reinterpret_cast<intptr_t>(context));
    if (level != Always)
    {
        const wchar_t* name = LogLevelToString(level);
        if (name != 0)
            std::swprintf(buffer + std::wcslen(buffer), capacity, L"%ls|", name);
    }
}

}
}
