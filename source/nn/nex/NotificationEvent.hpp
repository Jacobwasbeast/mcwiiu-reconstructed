#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP

#include <cstdint>

namespace nn { namespace nex {

class NotificationEvent
{
public:
    NotificationEvent();
    NotificationEvent(const NotificationEvent& other);
    ~NotificationEvent();

    std::uint32_t GetType() const;
    std::uint32_t GetParam1() const;
    std::uint32_t GetParam2() const;
    std::uint32_t GetParam3() const;

private:
    std::uint8_t m_flags;
    std::uint8_t m_reserved[3];
    std::uint32_t m_type;
    std::uint32_t m_param1;
    std::uint32_t m_param2;
    std::uint64_t m_identifier;
    std::uint32_t m_param3;
};

}}

#endif
