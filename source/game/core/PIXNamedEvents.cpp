#include "game/core/PIXNamedEvents.hpp"

#include <cstdarg>

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
