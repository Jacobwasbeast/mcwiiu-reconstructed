#ifndef GAME_CORE_PIXNAMEDEVENTS_HPP
#define GAME_CORE_PIXNAMEDEVENTS_HPP

void PIXAddNamedCounter(float colour, const char* name, ...);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
void PIXSetMarkerDeprecated(float colour, const char* name, ...);

#endif
