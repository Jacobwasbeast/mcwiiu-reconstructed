#ifndef GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP
#define GAME_CLIENT_AUDIO_EVENTSYSTEM_HPP

#include <cstdint>

class EventSystem
{
public:
    struct Handle
    {
        std::uint32_t index;
        std::uint32_t generation;
    };

    unsigned char m_reserved[0x58];
    Handle m_handle;
};

class SoundInstance;

int FindMarker__FP11EventSystemP13SoundInstanceiPCcPiT5(EventSystem* eventSystem,
                                                          SoundInstance* soundInstance,
                                                          int markerList,
                                                          char* markerName,
                                                          int* markerIndex,
                                                          int* markerType);

#endif
