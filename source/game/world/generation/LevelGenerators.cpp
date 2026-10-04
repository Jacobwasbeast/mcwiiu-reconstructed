#include "game/world/generation/LevelGenerators.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/world/LevelGenerationOptions.hpp"

LevelGenerators::LevelGenerators()
{
}

void LevelGenerators::removeLevelGenerator(LevelGenerationOptions* levelGenerator)
{
    std::vector<LevelGenerationOptions*>::iterator it;
    for (;;)
    {
        for (it = mLevelGenerators.begin(); it != mLevelGenerators.end() && *it != levelGenerator; ++it)
        {
        }
        if (it == mLevelGenerators.end())
        {
            break;
        }
        mLevelGenerators.erase(it);
    }
}

std::vector<LevelGenerationOptions*>* LevelGenerators::getLevelGenerators()
{
    return &mLevelGenerators;
}

bool LevelGenerators::SortLevelGenerators(const LevelGenerationOptions* first, const LevelGenerationOptions* second)
{
    const unsigned int firstPackId = first->getPack()->GetPackId();
    const unsigned int secondPackId = second->getPack()->GetPackId();
    if (firstPackId == 4)
    {
        const unsigned int themeId = first->getThemeId();
        if (themeId == 0 || themeId == static_cast<unsigned int>(-1))
        {
            return true;
        }
    }
    bool compareSortIndex = true;
    if (secondPackId == 4)
    {
        const unsigned int themeId = second->getThemeId();
        if (themeId == 0 || themeId == static_cast<unsigned int>(-1))
        {
            compareSortIndex = false;
        }
    }
    if (compareSortIndex)
    {
        cDLCInfo* dlcInfo = (cDLCInfo*)(unsigned long)gApp.GetDLCInfoInstance();
        DLC_INFO* firstInfo = dlcInfo->GetDLCInfoForPackID(firstPackId);
        dlcInfo = (cDLCInfo*)(unsigned long)gApp.GetDLCInfoInstance();
        DLC_INFO* secondInfo = dlcInfo->GetDLCInfoForPackID(secondPackId);
        if (firstInfo != NULL && secondInfo != NULL)
        {
            return secondInfo->SortIndex < firstInfo->SortIndex;
        }
    }
    return false;
}
