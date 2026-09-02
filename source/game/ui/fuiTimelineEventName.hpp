#ifndef GAME_UI_FUITIMELINEEVENTNAME_HPP
#define GAME_UI_FUITIMELINEEVENTNAME_HPP

struct fuiTimelineEventName
{
    char name[64];

    void EndianSwap();
    static void EndianSwap(void* pData);
};

#endif
