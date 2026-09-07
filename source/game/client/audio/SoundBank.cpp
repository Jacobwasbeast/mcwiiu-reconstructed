#include "game/client/audio/SoundBank.hpp"

#include "game/util/LegacyUtilityFunctions.hpp"

#include <cstdint>

std::int32_t FindAsset__FP9SoundBankPCcPQ2_9SoundBank10AssetEntryi(
    SoundBank* soundBank, const char* name, SoundBank::AssetEntry* entries, std::int32_t count)
{
    while (count != 0)
    {
        const std::int32_t midpoint = count / 2;
        SoundBank::AssetEntry* entry = entries + midpoint;
        const char* entryName = reinterpret_cast<const char*>(soundBank) + entry->nameOffset;
        const char* searchName = name;
        std::int32_t comparison = 0;

        while (true)
        {
            const std::int32_t entryCharacter = ourtoupper__Fc(*entryName);
            const std::int32_t searchCharacter = ourtoupper__Fc(*searchName);

            if (searchCharacter < entryCharacter)
            {
                comparison = -1;
                break;
            }
            if (entryCharacter < searchCharacter)
            {
                comparison = 1;
                break;
            }
            if (entryCharacter == 0)
            {
                return static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(entry));
            }

            ++entryName;
            ++searchName;
        }

        if (count == 1)
        {
            return 0;
        }

        if (comparison < 1)
        {
            entries = entry + 1;
            count = count - midpoint - 1;
        }
        else
        {
            count = midpoint;
        }
    }

    return 0;
}
