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
        it = mLevelGenerators.begin();
        while (it != mLevelGenerators.end() && *it != levelGenerator)
        {
            ++it;
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
    const unsigned int firstPackId = first->m_pPack->GetPackId();
    const unsigned int secondPackId = second->m_pPack->GetPackId();
    if (firstPackId == 4 && (first->getThemeId() == 0 || first->getThemeId() == -1))
    {
        return true;
    }
    if (secondPackId != 4 || (second->getThemeId() != 0 && second->getThemeId() != -1))
    {
        cDLCInfo* dlcInfo = (cDLCInfo*)(unsigned long)theApp->GetDLCInfoInstance();
        DLC_INFO* firstInfo = dlcInfo->GetDLCInfoForPackID(firstPackId);
        dlcInfo = (cDLCInfo*)(unsigned long)theApp->GetDLCInfoInstance();
        DLC_INFO* secondInfo = dlcInfo->GetDLCInfoForPackID(secondPackId);
        if (firstInfo != NULL && secondInfo != NULL)
        {
            return secondInfo->SortIndex < firstInfo->SortIndex;
        }
    }
    return false;
}
