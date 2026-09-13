#include "game/core/PIXNamedEvents.hpp"

#include <cstdarg>

struct _XPARTY_USER_LIST;

unsigned int XPartyGetUserList(_XPARTY_USER_LIST* users)
{
    return 0;
}

void PIXAddNamedCounter(float colour, const char* name, ...)
{
    va_list args;
    va_start(args, name);
    (void)colour;
    va_end(args);
}

void PIXBeginNamedEvent(float colour, const char* name, ...)
{
    va_list args;
    va_start(args, name);
    (void)colour;
    va_end(args);
}

void PIXEndNamedEvent()
{
}

void PIXSetMarkerDeprecated(float colour, const char* name, ...)
{
}
