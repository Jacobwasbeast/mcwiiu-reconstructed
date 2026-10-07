#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENT_HPP

#include <stdint.h>

#include "nn/nex/_DDL_NotificationEvent.hpp"

namespace nn
{
namespace nex
{

class StringStream;

class NotificationEvent : public _DDL_NotificationEvent
{
public:
    NotificationEvent();
    NotificationEvent(const NotificationEvent& other);

    uint32_t GetType() const;
    uint32_t GetParam1() const;
    uint32_t GetParam2() const;
    uint32_t GetParam3() const;
};

StringStream& operator<<(StringStream& stream, const NotificationEvent& event);

}
}

#endif
