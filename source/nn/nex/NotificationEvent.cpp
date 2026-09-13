#include "nn/nex/NotificationEvent.hpp"

namespace nn
{
namespace nex
{

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

std::uint32_t NotificationEvent::GetType() const
{
    return m_uiType;
}

std::uint32_t NotificationEvent::GetParam1() const
{
    return m_uiParam1;
}

std::uint32_t NotificationEvent::GetParam2() const
{
    return m_uiParam2;
}

std::uint32_t NotificationEvent::GetParam3() const
{
    return m_uiParam3;
}

}
}
