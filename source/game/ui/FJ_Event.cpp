#include "game/ui/FJ_Event.hpp"

FJ_Event::FJ_Event(eFJEventType eType, bool bBubbles, bool bCancelable)
    : m_eType(eType)
    , m_bBubbles(bBubbles)
    , m_bCancelable(bCancelable)
{
}
