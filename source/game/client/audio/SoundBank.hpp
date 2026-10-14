#ifndef GAME_CLIENT_AUDIO_SOUNDBANK_HPP
#define GAME_CLIENT_AUDIO_SOUNDBANK_HPP

#include <stdint.h>

struct SoundBank
{
    struct AssetEntry
    {
        int32_t nameOffset;
        int32_t value;
    };
};

int32_t FindAsset(SoundBank* soundBank, const char* name, SoundBank::AssetEntry* entries, int32_t count);

#endif
