#ifndef GAME_CLIENT_AUDIO_SOUNDBANK_HPP
#define GAME_CLIENT_AUDIO_SOUNDBANK_HPP

#include <cstdint>

struct SoundBank
{
    struct AssetEntry
    {
        std::int32_t nameOffset;
        std::int32_t value;
    };
};

std::int32_t FindAsset(
    SoundBank* soundBank, const char* name, SoundBank::AssetEntry* entries, std::int32_t count);

#endif
