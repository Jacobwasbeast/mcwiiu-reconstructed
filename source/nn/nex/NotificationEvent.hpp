#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP

#include <cstdint>

#include "nn/nex/_DDL_NotificationEvent.hpp"

namespace nn
{
namespace nex
{

class NotificationEvent : public _DDL_NotificationEvent
{
public:
    NotificationEvent();
    NotificationEvent(const NotificationEvent& other);
    virtual ~NotificationEvent();

    std::uint32_t GetType() const;
    std::uint32_t GetParam1() const;
    std::uint32_t GetParam2() const;
    std::uint32_t GetParam3() const;
};

}
}

#endif
