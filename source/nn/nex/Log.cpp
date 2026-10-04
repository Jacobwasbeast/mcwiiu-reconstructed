#include "nn/nex/Log.hpp"

#include <cstring>

#include "nn/nex/LogDeviceConsole.hpp"

namespace nn
{
namespace nex
{

Log::Log()
    : m_logDevice(new LogDeviceConsole(0)), m_outputFormat(new OutputFormat), m_logFilter(0)
{
}

Log::~Log()
{
    delete m_outputFormat;
    delete m_logDevice;
    delete m_logFilter;
}

void Log::SetLogDevice(LogDevice* device, bool deleteOld)
{
    LogDevice* old = m_logDevice;
    m_logDevice = device;
    if (old != 0 && deleteOld)
        delete old;
}

void Log::SetOutputFormat(OutputFormat* format)
{
    delete m_outputFormat;
    m_outputFormat = format;
}

void Log::SetLogFilter(LogFilter* filter)
{
    delete m_logFilter;
    m_logFilter = filter;
}

OutputFormat* Log::GetOutputFormat()
{
    return m_outputFormat;
}

void Log::PrepareOutput(wchar_t* buffer, unsigned int capacity, void* context)
{
    capacity -= 2;
    m_outputFormat->StartString(buffer, capacity);
    m_outputFormat->StartPrefixes(buffer, capacity);
    AddCustomPrefix(buffer, capacity, context);
    m_outputFormat->AddPrefixes(buffer, capacity);
    m_outputFormat->EndPrefixes(buffer, capacity);
    m_outputFormat->AddIndent(buffer, capacity);
}

void Log::OutputImpl(const wchar_t* format, va_list arguments, void* context)
{
    if (m_logDevice != 0 && (m_logFilter == 0 || m_logFilter->OutputCurrentTrace()) &&
        m_outputFormat != 0)
    {
        wchar_t buffer[256];
        std::memset(buffer, 0, sizeof(buffer));
        PrepareOutput(buffer, 256, context);
        m_outputFormat->AddMessage(buffer, 255, format, arguments);
        m_outputFormat->EndString(buffer, 254);
        m_logDevice->Output(buffer);
    }
}

void Log::Output(const wchar_t* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    OutputImpl(format, arguments, 0);
    va_end(arguments);
}

void Log::OutputString(const wchar_t* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    OutputImpl(format, arguments, 0);
    va_end(arguments);
}

}
}
