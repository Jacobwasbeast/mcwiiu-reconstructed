#ifndef GAME_CLIENT_AUDIO_FALLOFFGRAPH_HPP
#define GAME_CLIENT_AUDIO_FALLOFFGRAPH_HPP

#include "game/core/Handle.hpp"

struct _SAMPLE
{
    unsigned char reserved0[0x208];
    Handle falloffHandle;
    unsigned char reserved1[0x5f4];
    int falloffApplied;
    unsigned char reserved2[0x48];
    int falloffMode;
};

struct FalloffGraphPoint
{
    float values[8];
};

struct FalloffGraph
{
    int reserved;
    int mode;
    int volumePointCount;
    int exclusionPointCount;
    int lowpassPointCount;
    FalloffGraphPoint points[1];
};

void FalloffGraph_Set(_SAMPLE* sample, int flags, FalloffGraph* graph);

#endif
