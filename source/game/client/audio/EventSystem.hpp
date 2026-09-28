#ifndef GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP
#define GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP

#include <stdint.h>

class EventSystem
{
public:
    struct Handle
    {
        uint32_t index;
        uint32_t generation;
    };

    unsigned char m_reserved[0x58];
    Handle m_handle;
};

class SoundInstance;

int FindMarker(EventSystem* eventSystem,
                                                          SoundInstance* soundInstance,
                                                          int markerList,
                                                          char* markerName,
                                                          int* markerIndex,
                                                          int* markerType);

#endif
