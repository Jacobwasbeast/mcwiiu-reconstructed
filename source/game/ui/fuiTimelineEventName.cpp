#include "game/ui/fuiTimelineEventName.hpp"

void fuiTimelineEventName::EndianSwap(void* pData)
{
    static_cast<fuiTimelineEventName*>(pData)->EndianSwap();
}
