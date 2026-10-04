#include "nn/nex/OutputFormat.hpp"

#include <cwchar>

#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

OutputFormat::OutputFormat()
    : m_indent(0), m_field4(0), m_flags(), m_field14(0),
      m_startTime(static_cast<uint32_t>(Time::GetTime()))
{
}

OutputFormat::~OutputFormat()
{
}

void OutputFormat::StartString(wchar_t* buffer, unsigned int)
{
    buffer[0] = 0;
}

void OutputFormat::AppendToString(wchar_t* buffer, const wchar_t* text, unsigned int capacity)
{
    const unsigned int length = static_cast<unsigned int>(std::wcslen(buffer));
    const unsigned int remaining = capacity - length;
    if (remaining != 0)
        std::swprintf(buffer + length, remaining, L"%ls", text);
}

void OutputFormat::StartPrefixes(wchar_t* buffer, unsigned int capacity)
{
    AppendToString(buffer, L"(", capacity);
}

void OutputFormat::AddPrefixes(wchar_t*, unsigned int)
{
}

void OutputFormat::EndPrefixes(wchar_t*, unsigned int)
{
}

void OutputFormat::AddIndent(wchar_t*, unsigned int)
{
}

unsigned int OutputFormat::AddMessageImpl(wchar_t*, unsigned int, const wchar_t*, va_list)
{
    return 0;
}

void OutputFormat::AddMessage(wchar_t* buffer, unsigned int capacity,
                              const wchar_t* format, va_list arguments)
{
    AddMessageImpl(buffer, capacity, format, arguments);
}

void OutputFormat::IncreaseIndent(unsigned int amount)
{
    m_indent += amount;
}

void OutputFormat::DecreaseIndent(unsigned int amount)
{
    m_indent = amount < m_indent ? m_indent - amount : 0;
}

void OutputFormat::EndString(wchar_t*, unsigned int)
{
}

}
}
