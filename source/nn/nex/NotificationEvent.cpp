#include "nn/nex/NotificationEvent.hpp"

namespace nn { namespace nex {

NotificationEvent::NotificationEvent()
    : m_flags(0), m_reserved(), m_type(0), m_param1(0), m_param2(0), m_identifier(0), m_param3(0)
{
}

NotificationEvent::NotificationEvent(const NotificationEvent& other)
    : m_flags(other.m_flags), m_reserved(), m_type(other.m_type), m_param1(other.m_param1),
      m_param2(other.m_param2), m_identifier(other.m_identifier), m_param3(other.m_param3)
{
}

NotificationEvent::~NotificationEvent()
{
}

std::uint32_t NotificationEvent::GetType() const
{
    return m_type;
}

std::uint32_t NotificationEvent::GetParam1() const
{
    return m_param1;
}

std::uint32_t NotificationEvent::GetParam2() const
{
    return m_param2;
}

std::uint32_t NotificationEvent::GetParam3() const
{
    return m_param3;
}

}}
