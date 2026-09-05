#include "game/world/generation/PlayersEnteredVolumeCondition.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"

class ServerPlayer;

PlayersEnteredVolumeCondition::PlayersEnteredVolumeCondition(const XmlObject& object)
    : IntegerCondition(object)
    , m_volumeName(object[L"volumeName"].maybeString())
    , m_useTargetAreas(object[L"useTargetAreas"].maybeBool())
    , m_useRatio(object[L"useRatio"].maybeBool())
{
}

PlayersEnteredVolumeCondition::~PlayersEnteredVolumeCondition()
{
}

int PlayersEnteredVolumeCondition::GetValue(MasterGameMode* gameMode) const
{
    std::vector<boost::shared_ptr<ServerPlayer> > players;
    if (CommonMasterGameMode::GetMiniGame()->GetId() == 3)
        players = gameMode->GetGlidePlayers();
    else
        players = gameMode->GetPlayers(true, false);

    int entered = 0;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        const StatsUID uid = (*it)->getSUID();
        if (gameMode->mGameStats.GetEndZoneEntered(uid) != 0)
        {
            ++entered;
            continue;
        }

        if (!m_useTargetAreas.isSet() || !m_useTargetAreas.get())
        {
            LevelGenerationOptions* options = CommonMasterGameMode::GetMasterOrClient()->getCurrentLevelGenOptions();
            const std::wstring volumeName = m_volumeName.isSet() ? m_volumeName.get() : std::wstring();
            AABB* area = options->getRequiredGameRules()->getNamedArea(volumeName);
            if (area != NULL && (*it)->getBoundingBox()->intersects(area->mMinX, area->mMinY, area->mMinZ,
                                                                     area->mMaxX, area->mMaxY, area->mMaxZ))
                ++entered;
        }
        else
        {
            std::vector<TargetAreaRuleDefinition*> areas;
            LevelGenerationOptions* options = CommonMasterGameMode::GetMasterOrClient()->getCurrentLevelGenOptions();
            options->getRequiredGameRules()->getTargetAreas(&areas);
            for (std::vector<TargetAreaRuleDefinition*>::const_iterator area = areas.begin(); area != areas.end(); ++area)
            {
                if (!(*area)->mPrimary || gameMode->mGameStats.GetLapsCompleted(uid) < static_cast<unsigned int>((*area)->getSize()))
                    continue;
                AABB* volume = (*area)->getArea();
                if (volume != NULL && (*it)->getBoundingBox()->intersects(volume->mMinX, volume->mMinY, volume->mMinZ,
                                                                           volume->mMaxX, volume->mMaxY, volume->mMaxZ) &&
                    gameMode->mGameStats.GetLastCheckpointSequenceID(uid) == gameMode->mCurrentCheckpoint)
                {
                    ++entered;
                    break;
                }
            }
        }
    }
    return entered;
}

int PlayersEnteredVolumeCondition::GetThresholdPlayerOverride(MasterGameMode* gameMode, int playerCount) const
{
    if (m_option >= 0)
        return CommonMasterGameMode::GetMiniGame()->HostOption_Time(static_cast<eCustomGameModeHostOption>(m_option));
    if (m_useRatio.isSet() && m_useRatio.get())
    {
        if (m_threshold == 1)
            return playerCount;
        int threshold = playerCount / m_threshold + 1;
        return threshold < m_threshold ? threshold : m_threshold;
    }
    return m_threshold;
}

unsigned int PlayersEnteredVolumeCondition::GetThreshold(MasterGameMode* gameMode) const
{
    int playerCount;
    if (CommonMasterGameMode::GetMiniGame()->GetId() == 3)
        playerCount = static_cast<int>(gameMode->GetGlidePlayers().size());
    else
        playerCount = static_cast<int>(gameMode->GetPlayers(true, false).size());
    return static_cast<unsigned int>(GetThresholdPlayerOverride(gameMode, playerCount));
}

void PlayersEnteredVolumeCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(GetValue(gameMode));
    params->push_back(0x57a4fda9);
}

bool PlayersEnteredVolumeCondition::IsUsingPrimaryTargetAreas() const
{
    return m_useTargetAreas.isSet() ? m_useTargetAreas.get() : false;
}

bool PlayersEnteredVolumeCondition::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    return true;
}
