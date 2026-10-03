#ifndef GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP
#define GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP

#include "game/core/Handle.hpp"

class EventSystem
{
public:
    unsigned char m_reserved[0x58];
    Handle m_handle;
};

class SoundInstance;

int Var_LookupWithContext(EventSystem* eventSystem, Handle handle, const char* name, int flags,
                          void* value);

int FindMarker(EventSystem* eventSystem, SoundInstance* soundInstance, int markerList, const char* markerName,
               int* markerIndex, int* markerType);

#endif
