#include "game/client/audio/SoundBank.hpp"

#include "game/util/LegacyUtilityFunctions.hpp"

#include <stdint.h>

int32_t FindAsset(
    SoundBank* soundBank, const char* name, SoundBank::AssetEntry* entries, int32_t count)
{
    while (count != 0)
    {
        const int32_t midpoint = count / 2;
        SoundBank::AssetEntry* entry = entries + midpoint;
        const char* entryName = reinterpret_cast<const char*>(soundBank) + entry->nameOffset;
        const char* searchName = name;
        int32_t comparison = 0;

        while (true)
        {
            const int32_t entryCharacter = ourtoupper(*entryName);
            const int32_t searchCharacter = ourtoupper(*searchName);

            if (searchCharacter < entryCharacter)
            {
                comparison = 1;
                break;
            }
            if (entryCharacter < searchCharacter)
            {
                comparison = -1;
                break;
            }
            if (entryCharacter == 0)
            {
                return static_cast<int32_t>(reinterpret_cast<uintptr_t>(entry));
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
