#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

StringStream& operator<<(StringStream& stream, const NotificationEvent& event)
{
    stream << L"NotificationEvent (" << L" PIDSource=";
    stream.StreamNumber(event.m_uiValue);
    stream << L" Type=";
    stream.StreamNumber(event.m_uiType / 1000);
    stream << L" SubType=";
    stream.StreamNumber(event.m_uiType % 1000);
    stream << L" Param1=";
    stream.StreamNumber(event.m_uiParam1);
    stream << L" Param2=";
    stream.StreamNumber(event.m_uiParam2);
    stream << L" Param3=";
    stream.StreamNumber(event.m_uiParam3);
    stream << L" UInt64Param1=";
    stream << ((static_cast<unsigned long long>(event.m_uiParam1) << 32) | event.m_uiParam2);
    stream << L" StrParam=" << static_cast<const wchar_t*>(event.m_StringParam) << L") ";
    return stream;
}

NotificationEvent::NotificationEvent()
    : _DDL_NotificationEvent()
{
    m_uiValue = 0;
    m_uiType = 0;
    m_uiParam1 = 0;
    m_uiParam2 = 0;
    m_uiParam3 = 0;
}

NotificationEvent::NotificationEvent(const NotificationEvent& other)
    : _DDL_NotificationEvent(other)
{
}

NotificationEvent::~NotificationEvent()
{
}

uint32_t NotificationEvent::GetType() const
{
    return m_uiType;
}

uint32_t NotificationEvent::GetParam1() const
{
    return m_uiParam1;
}

uint32_t NotificationEvent::GetParam2() const
{
    return m_uiParam2;
}

uint32_t NotificationEvent::GetParam3() const
{
    return m_uiParam3;
}

}
}
