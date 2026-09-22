#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include <algorithm>
#include <cmath>
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConditionConnective.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GameCondition.hpp"
#include "game/core/GameMode.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/PowerupRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/PlayersEnteredVolumeCondition.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/core/StatsUID.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MapGenerator.hpp"
#include "game/world/generation/TumbleGenerator.hpp"
#include "game/world/generation/WallGenerator.hpp"
#include "game/world/generation/LayerGenerationRuleDefinition.hpp"
#include "game/util/Random.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"

CommonMasterGameMode* CommonMasterGameMode::GetMasterOrClient()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    return minecraft == NULL ? NULL : minecraft->GetClientMasterGameMode();
}
CommonMasterGameMode::CommonMasterGameMode()
    : mCurrentLevelGenOptions(NULL), mNextLevelGenOptions(NULL), mLocalizedStringTable(new StringTable()), mEnglishStringTable(new StringTable()), mRoundInfo(), mState(0), mNextState(0), mGeneratorSeed(-1), mGeneratorSeedScale(1.0f), mMapGenerator(NULL), mChosenThemeWordId(0xffffffffU), mCheckpointCount(0), mCurrentCheckpoint(0), mDistancesCached(false), mPrimaryTargetActive(false), mUsingTrialContent(false), mBuildOffVotes(false)
{
    InitializeCriticalSection(&mGeneratorMutex);
}
CommonMasterGameMode::~CommonMasterGameMode()
{
    delete mLocalizedStringTable;
    delete mEnglishStringTable;
    delete mMapGenerator;
    DeleteCriticalSection(&mGeneratorMutex);
}
void CommonMasterGameMode::Setup(EMiniGameId miniGameId)
{
    if (miniGameId == eMiniGameId_Build)
    {
        std::wstring archiveName(L"gameModeLanguages.loc");
        if (gApp.hasArchiveFile(archiveName, false))
        {
            delete mLocalizedStringTable;
            delete mEnglishStringTable;
            arrayWithLength<unsigned char> data = gApp.getArchiveFile(archiveName, false);
            mLocalizedStringTable = new StringTable(data, false, false);
            mEnglishStringTable = new StringTable(data, false, false);
            mEnglishStringTable->SelectLanguage(L"en_EN");
        }
    }
}
void CommonMasterGameMode::Tick()
{
    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL && mState > 3 && (mState < 8 || mState == 10))
        mMapGenerator->ContinueGeneration();
    LeaveCriticalSection(&mGeneratorMutex);
}
const MiniGameDef* CommonMasterGameMode::GetMiniGame()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    return minecraft == NULL ? NULL : minecraft->GetMiniGame();
}
void CommonMasterGameMode::SetupMiniGameInstance(const MiniGameDef& definition, int variant)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL) minecraft->SetupMiniGameInstance(definition, variant);
}
void CommonMasterGameMode::OnExitedGame() { mRoundInfo.reset(); }
unsigned int CommonMasterGameMode::GetNextGameMode() const
{
    return mCurrentLevelGenOptions == NULL ? 0 : mNextState;
}
AABB* CommonMasterGameMode::GetTeamArea(Team*) const { return mTeamAreas.empty() ? NULL : mTeamAreas.front(); }
Team* CommonMasterGameMode::GetTeamAtPosition(double x, double y, double z, double radius) const
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Level* level = minecraft == NULL ? NULL : minecraft->getLevel(0);
    if (level == NULL) return NULL;

    Scoreboard* scoreboard = level->GetScoreboard();
    if (scoreboard == NULL) return NULL;
    std::vector<PlayerTeam*>* teams = scoreboard->getPlayerTeams();
    if (teams == NULL) return NULL;

    Team* result = NULL;
    for (std::vector<PlayerTeam*>::const_iterator it = teams->begin(); it != teams->end(); ++it)
    {
        Team* team = *it;
        AABB* area = GetTeamArea(team);
        if (area != NULL)
        {
            AABB* expanded = area->grow(radius);
            if (expanded->contains(x, y, z))
            {
                result = team;
                break;
            }
        }
    }
    delete teams;
    return result;
}
bool CommonMasterGameMode::HasFallenOffLayer(const double& x, const double& z)
{
    EnterCriticalSection(&mGeneratorMutex);
    bool result = false;
    if (mMapGenerator != NULL)
    {
        int layerCount = mMapGenerator->GetNumberOfLayers();
        for (int layer = 0; layer < layerCount; ++layer)
        {
            float minimum;
            float maximum;
            mMapGenerator->GetLayerExtents(static_cast<unsigned int>(layer), &minimum, &maximum);
            float threshold = minimum + 0.8f;
            if (threshold <= x && z < threshold)
            {
                result = true;
                break;
            }
        }
    }
    LeaveCriticalSection(&mGeneratorMutex);
    return result;
}
bool CommonMasterGameMode::IsOnGeneratedLayer(const int& layer, const double& y)
{
    return layer >= 0 && static_cast<unsigned int>(layer) < mLayerAreas.size() && y >= 0.0;
}
int CommonMasterGameMode::GetLayer(const double& y)
{
    EnterCriticalSection(&mGeneratorMutex);
    int result = -1;
    if (mMapGenerator != NULL)
    {
        int layerCount = mMapGenerator->GetNumberOfLayers();
        for (int layer = 0; layer < layerCount; ++layer)
        {
            AABB* area = mMapGenerator->GetLayerBB(static_cast<unsigned int>(layer));
            if (area != NULL && area->mMinY <= y && y < area->mMaxY)
            {
                result = layer;
                break;
            }
        }
    }
    LeaveCriticalSection(&mGeneratorMutex);
    return result;
}
unsigned int CommonMasterGameMode::GetLayerCount() { return static_cast<unsigned int>(mLayerAreas.size()); }
AABB* CommonMasterGameMode::GetLayerAABB(const int& layer)
{
    return layer < 0 || static_cast<unsigned int>(layer) >= mLayerAreas.size() ? NULL : mLayerAreas[static_cast<unsigned int>(layer)];
}
void CommonMasterGameMode::UpdateGeneratorSeed(long long seed, float scale) { mGeneratorSeed = seed; mGeneratorSeedScale = scale; }
void CommonMasterGameMode::RestartMapGenerator()
{
    EnterCriticalSection(&mGeneratorMutex);

    LevelRuleset* rules = gApp.getLevelGenerationOptions()->getRequiredGameRules();
    long long seed = mGeneratorSeed;
    mGeneratorSeed = -1;
    Random random(seed);

    int miniGameId = GetMiniGame()->GetId();
    if (miniGameId == 2)
    {
        LayerGenerationRuleDefinition* layerRule = static_cast<LayerGenerationRuleDefinition*>(rules->getRuleByType(static_cast<ConsoleGameRules::EGameRuleType>(0x26)));
        AABB* area = rules->getNamedArea(L"LevelGeneration");
        if (mMapGenerator != NULL) delete mMapGenerator;

        Random tumbleRandom(seed);
        unsigned int layerCount = GetMiniGame()->GetLayerCount();
        std::vector<LayerGenerator*>* layers = layerRule->CreateLayers(tumbleRandom, area, layerCount, mGeneratorSeedScale);

        TumbleGenerator* tumble = new TumbleGenerator(tumbleRandom, *layers);
        mMapGenerator = tumble;
        Level* level = GeneratorTargetLevel();
        mMapGenerator->StartGeneration(level);

        if (layers != NULL)
        {
            delete layers;
            LeaveCriticalSection(&mGeneratorMutex);
            return;
        }
    }
    else
    {
        if (miniGameId != 4)
        {
            LeaveCriticalSection(&mGeneratorMutex);
            return;
        }

        std::vector<AABB*> areas;
        rules->getNamedAreas(0xff, &areas);
        std::vector<AABB*> walls;
        rules->getNamedAreas(0xff00, &walls);

        if (mMapGenerator != NULL) delete mMapGenerator;

        WallGenerator* wall = new WallGenerator(random, &areas, &walls);
        mMapGenerator = wall;
        Level* level = GeneratorTargetLevel();
        mMapGenerator->StartGeneration(level);
    }

    LeaveCriticalSection(&mGeneratorMutex);
}
void CommonMasterGameMode::RemoveMapGenerator() { delete mMapGenerator; mMapGenerator = NULL; }
void CommonMasterGameMode::CleanUpMapGenerator() { RemoveMapGenerator(); }
void CommonMasterGameMode::clearBuildOffVotes()
{
    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        mVoteables->getVoteable(2)->clearVotes();
        mVoteables->getVoteable(3)->clearVotes();
    }
}
std::wstring CommonMasterGameMode::getThemeWordEnglish(int id)
{
    const wchar_t* value = mEnglishStringTable == NULL ? NULL : mEnglishStringTable->getString(id);
    return value == NULL ? std::wstring() : std::wstring(value);
}
LevelGenerationOptions* CommonMasterGameMode::getRoundLevelGenOptions() const
{
    if (mCurrentLevelGenOptions != NULL && !mCurrentLevelGenOptions->isGameModeLobby())
        return mCurrentLevelGenOptions;
    if (mNextLevelGenOptions != NULL && !mNextLevelGenOptions->isGameModeLobby())
        return mNextLevelGenOptions;
    return gApp.getLevelGenerationOptions();
}
unsigned int CommonMasterGameMode::CheckpointIDtoSequenceID(int id) const
{
    return id < 0 || static_cast<unsigned int>(id) >= mCheckpoints.size() || mCheckpoints[id] == NULL ? 0 : mCheckpoints[id]->getSequenceID();
}
void CommonMasterGameMode::setCheckpoints(const std::vector<CheckpointRuleDefinition*>& checkpoints)
{
    mCheckpoints = checkpoints;
    mCheckpointCount = 0;
    for (std::vector<CheckpointRuleDefinition*>::const_iterator it = mCheckpoints.begin(); it != mCheckpoints.end(); ++it)
        if (*it != NULL) mCheckpointCount = std::max(mCheckpointCount, (*it)->getSequenceID());
}
double CommonMasterGameMode::getTotalDistance()
{
    if (!mDistancesCached) CacheMapDistance();
    return mTotalDistance;
}
std::vector<double>* CommonMasterGameMode::getCheckpointDistances()
{
    if (!mDistancesCached) CacheMapDistance();
    return &mCheckpointDistances;
}
void CommonMasterGameMode::ResetMapDistances()
{
    mCurrentCheckpoint = 0;
    mTotalDistance = 0.0;
    mCheckpointDistances.clear();
    mLayerDistances.clear();
    mDistancesCached = false;
}
void CommonMasterGameMode::CacheMapDistance()
{
    ResetMapDistances();

    std::vector<Vec3*> spawnPositions;
    LevelRuleset* ruleDefinitions = gApp.getGameRuleDefinitions();
    if (ruleDefinitions != NULL)
    {
        GameRuleDefinition* spawnRule = ruleDefinitions->getRuleByType(static_cast<ConsoleGameRules::EGameRuleType>(0x1d));
        if (spawnRule != NULL)
        {
            bool allSpawns = !GetMiniGame()->HostOption_Bool(static_cast<eCustomGameModeHostOption>(4));
            (void)allSpawns;
        }
    }

    if (mCheckpoints.empty())
    {
        double farthestDistance = 0.0;
        mCheckpointDistances.push_back(farthestDistance);
        mLayerDistances.push_back(farthestDistance);
        mTotalDistance += farthestDistance;
    }
    else
    {
        double closestSpawnDistance = 0.0;
        std::vector<CheckpointRuleDefinition*>::const_iterator it = mCheckpoints.begin();
        if (it != mCheckpoints.end())
        {
            do
            {
                if ((*it)->getSequenceID() == 0)
                {
                    std::vector<Vec3*>::const_iterator spawnIt = spawnPositions.begin();
                    if (spawnIt != spawnPositions.end())
                    {
                        do
                        {
                            AABB* area = (*it)->getArea();
                            double distance = (*spawnIt)->distanceTo(area);
                            ++spawnIt;
                            if (closestSpawnDistance < distance) closestSpawnDistance = distance;
                        } while (spawnIt != spawnPositions.end());
                    }
                }
                ++it;
            } while (it != mCheckpoints.end());
        }
        mCheckpointDistances.push_back(closestSpawnDistance);
        mTotalDistance += closestSpawnDistance;

        double lapDistance = 0.0;
        int sequenceId = 0;
        bool done = false;
        do
        {
            int currentSequenceId = sequenceId;
            LevelGenerationOptions* nextOptions = gApp.getLevelGenerationOptions();
            LevelRuleset* nextRuleset = nextOptions->getRequiredGameRules();
            std::vector<CheckpointRuleDefinition*> nextCheckpoints = nextRuleset->getCheckpointsBySequenceID(currentSequenceId + 1);
            LevelGenerationOptions* currentOptions = gApp.getLevelGenerationOptions();
            LevelRuleset* currentRuleset = currentOptions->getRequiredGameRules();
            std::vector<CheckpointRuleDefinition*> currentCheckpoints = currentRuleset->getCheckpointsBySequenceID(currentSequenceId);

            if (nextCheckpoints.empty())
            {
                done = true;
            }
            else
            {
                bool hasSpawnPoint = false;
                double farthestPairDistance = 0.0;
                std::vector<CheckpointRuleDefinition*>::const_iterator nextIt = nextCheckpoints.begin();
                if (nextIt != nextCheckpoints.end())
                {
                    do
                    {
                        if (!hasSpawnPoint && (*nextIt)->hasSpawnPoint())
                            hasSpawnPoint = true;

                        std::vector<CheckpointRuleDefinition*>::const_iterator curIt = currentCheckpoints.begin();
                        if (curIt != currentCheckpoints.end())
                        {
                            do
                            {
                                AABB* currentArea = (*curIt)->getArea();
                                AABB* nextArea = (*nextIt)->getArea();
                                Vec3* nextCenter = nextArea->getCenter();
                                double distance = nextCenter->distanceTo(currentArea);
                                ++curIt;
                                if (farthestPairDistance < distance) farthestPairDistance = distance;
                            } while (curIt != currentCheckpoints.end());
                        }
                        ++nextIt;
                    } while (nextIt != nextCheckpoints.end());
                }

                double sequenceDistance = farthestPairDistance;
                lapDistance = lapDistance + farthestPairDistance;
                if (hasSpawnPoint)
                {
                    mLayerDistances.push_back(lapDistance);
                    lapDistance = 0.0;
                }
                mCheckpointDistances.push_back(sequenceDistance);
                mTotalDistance += farthestPairDistance;
            }
            sequenceId = currentSequenceId + 1;
        } while (!done);

        LevelGenerationOptions* lastOptions = gApp.getLevelGenerationOptions();
        LevelRuleset* lastRuleset = lastOptions->getRequiredGameRules();
        std::vector<CheckpointRuleDefinition*> lastCheckpoints = lastRuleset->getCheckpointsBySequenceID(sequenceId - 1);
        (void)lastCheckpoints;

        double finalDistance = 0.0;
        mCheckpointDistances.push_back(finalDistance);
        mTotalDistance += finalDistance;
    }

    LevelGenerationOptions* targetOptions = gApp.getLevelGenerationOptions();
    LevelRuleset* targetRuleset = targetOptions->getRequiredGameRules();
    std::vector<TargetAreaRuleDefinition*> targetAreas;
    targetRuleset->getTargetAreas(&targetAreas);

    mDistancesCached = true;
}
void CommonMasterGameMode::CheckProgress(boost::shared_ptr<Player> player, const std::vector<CheckpointRuleDefinition*>* rules, const std::vector<AABB*>* areas)
{
    if (!GetMiniGame()->HasFeatureCheckpoints())
        return;

    int checkpointId;
    int lapsCompleted;
    if (player->GetType() == 0xc800004)
    {
        boost::shared_ptr<GhostPlayer> ghost = boost::static_pointer_cast<GhostPlayer>(player);
        checkpointId = ghost->getLastCheckpointId();
        lapsCompleted = ghost->getLapsCompleted();
    }
    else
    {
        const StatsUID suid = player->getSUID();
        checkpointId = static_cast<int>(GetLastCheckpointID(suid));
        Scoreboard* scoreboard = getScoreboard();
        lapsCompleted = scoreboard->GetLapsCompleted(suid, -1);
    }

    unsigned int sequenceId = 0;
    if (checkpointId != -1)
    {
        CheckpointRuleDefinition* rule = rules->at(static_cast<unsigned int>(checkpointId));
        sequenceId = rule->getSequenceID() + 1;
    }

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    LevelRuleset* ruleset = options->getRequiredGameRules();
    std::vector<CheckpointRuleDefinition*> sequenceCheckpoints = ruleset->getCheckpointsBySequenceID(sequenceId);

    if (sequenceCheckpoints.empty() && lapsCompleted < static_cast<int>(mCurrentCheckpoint))
    {
        OnLapCompleted(player);
        LevelGenerationOptions* lapOptions = gApp.getLevelGenerationOptions();
        LevelRuleset* lapRuleset = lapOptions->getRequiredGameRules();
        sequenceCheckpoints = lapRuleset->getCheckpointsBySequenceID(0);
    }

    double closestDistance = 1.7976931348623157e+308;
    if (sequenceCheckpoints.empty())
    {
        std::vector<AABB*>::const_iterator it = areas->begin();
        if (it != areas->end())
        {
            do
            {
                double distance = player->getPos(1.0f)->distanceTo(*it);
                ++it;
                if (distance < closestDistance) closestDistance = distance;
            } while (it != areas->end());
        }
    }
    else
    {
        std::vector<CheckpointRuleDefinition*>::const_iterator it = sequenceCheckpoints.begin();
        if (it != sequenceCheckpoints.end())
        {
            do
            {
                AABB* area = (*it)->getArea();
                double distance = player->getPos(1.0f)->distanceTo(area);
                ++it;
                if (distance < closestDistance) closestDistance = distance;
            } while (it != sequenceCheckpoints.end());
        }
    }
    double remainingDistance = mCheckpointDistances.at(sequenceId) - closestDistance;

    double totalDistanceTravelled = remainingDistance;
    for (unsigned int i = 0; i < sequenceId; ++i)
        totalDistanceTravelled += mCheckpointDistances.at(i);

    double progress = (totalDistanceTravelled / mTotalDistance) * 100.0;

    OnProgressMade(player, progress);
}
void CommonMasterGameMode::CheckProgress(const std::vector<boost::shared_ptr<Player> >& players)
{
    if (!GetMiniGame()->HasFeatureCheckpoints())
        return;

    if (!mCheckpoints.empty() || !mLayerAreas.empty())
    {
        if (!mDistancesCached)
            CacheMapDistance();

        for (std::vector<boost::shared_ptr<Player> >::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            CheckProgress(*it, &mCheckpoints, &mLayerAreas);
        }
    }
}
void CommonMasterGameMode::OnLapCompleted(const boost::shared_ptr<Player>& player)
{
    if (player.get() == NULL)
        return;

    GameMode* gameMode = player->GetGameMode();
    if (gameMode != NULL)
    {
        gameMode->RecordLastCheckpoint(-1, -1, false, false, true);
        gameMode->RecordLapCompleted();
    }
}
void CommonMasterGameMode::CheckCheckpoints(const std::vector<boost::shared_ptr<Player> >& players) { CheckIntersectsNamedAreas(players, mCheckpoints, &CommonMasterGameMode::OnCheckpointEnter); }
void CommonMasterGameMode::CheckTargets(const std::vector<boost::shared_ptr<Player> >& players)
{
    std::vector<TargetAreaRuleDefinition*> targetAreas;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        LevelRuleset* rules = options->getRequiredGameRules();
        if (rules != NULL)
            rules->getTargetAreas(&targetAreas);
    }
    CheckIntersectsNamedAreas(players, targetAreas, &CommonMasterGameMode::OnTargetEnter);
}
void CommonMasterGameMode::CheckPowerups(const std::vector<boost::shared_ptr<Player> >& players)
{
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == NULL || !miniGame->ArePowerupsActive())
        return;

    std::vector<PowerupRuleDefinition*> powerups;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        LevelRuleset* rules = options->getRequiredGameRules();
        if (rules != NULL)
            rules->getPowerups(&powerups);
    }
    CheckIntersectsNamedAreas(players, powerups, &CommonMasterGameMode::OnPowerupEnter);
}
void CommonMasterGameMode::OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint, CheckpointRuleDefinition* rule)
{
    if (player.get() == NULL)
        return;

    GameMode* gameMode = player->GetGameMode();
    if (gameMode != NULL)
        gameMode->RecordLastCheckpoint(checkpoint, rule->getSequenceID(), rule->hasSpawnPoint(), rule->hasSpawnPoint(), false);
}
void CommonMasterGameMode::OnTargetEnter(const boost::shared_ptr<Player>& player, unsigned int target, TargetAreaRuleDefinition* rule)
{
    const MiniGameDef* miniGame = GetMiniGame();
    bool usingPrimaryTargetAreas = false;

    if (miniGame != NULL)
    {
        GameCondition* endConditions = reinterpret_cast<GameCondition*>(miniGame->GetEndConditions());
        if (endConditions != NULL)
        {
            ConditionConnective* connective = dynamic_cast<ConditionConnective*>(endConditions);
            if (connective != NULL)
            {
                const std::vector<GameCondition*>& conditions = connective->GetConditions();
                for (std::vector<GameCondition*>::const_iterator it = conditions.begin(); it != conditions.end(); ++it)
                {
                    PlayersEnteredVolumeCondition* volumeCondition = dynamic_cast<PlayersEnteredVolumeCondition*>(*it);
                    if (volumeCondition != NULL && volumeCondition->IsUsingPrimaryTargetAreas())
                    {
                        usingPrimaryTargetAreas = true;
                        break;
                    }
                }
            }
        }
    }

    Scoreboard* scoreboard = getScoreboard();
    const StatsUID suid = player->getSUID();

    if (!scoreboard->GetEndZoneEntered(suid, -1))
    {
        if (!player->GetGameMode()->isSpectator())
        {
            if (!rule->mPrimary && usingPrimaryTargetAreas)
            {
                if (miniGame != NULL && miniGame->ArePointsActive())
                    OnSecondaryTargetEnter(player, rule);
            }
            else if (rule->mLaps <= scoreboard->GetLapsCompleted(suid, -1))
            {
                if (scoreboard->GetLastCheckpointSequenceID(suid, -1) == static_cast<int>(mCurrentCheckpoint))
                    OnPrimaryTargetEnter(player, rule);
            }
        }
    }
}
void CommonMasterGameMode::OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int, PowerupRuleDefinition* rule)
{
    rule->GiveRandomPowerup(player);
}
void CommonMasterGameMode::OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
    (void)player;
    (void)rule;
}
void CommonMasterGameMode::OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
    (void)player;
    (void)rule;
}
void CommonMasterGameMode::OnProgressMade(const boost::shared_ptr<Player>& player, double progress)
{
    (void)player;
    (void)progress;
}
void CommonMasterGameMode::setChosenThemeWordId(int id) { mChosenThemeWordId = static_cast<unsigned int>(id); }
template<class Rule> void CommonMasterGameMode::CheckIntersectsNamedAreas(const std::vector<boost::shared_ptr<Player> >& players, const std::vector<Rule*>& rules, void (CommonMasterGameMode::*callback)(const boost::shared_ptr<Player>&, unsigned int, Rule*))
{
    if (rules.empty())
        return;

    for (typename std::vector<boost::shared_ptr<Player> >::const_iterator playerIt = players.begin(); playerIt != players.end(); ++playerIt)
    {
        const boost::shared_ptr<Player>& player = *playerIt;
        if (player->GetType() == 0xc800004 || player->isAlive())
        {
            unsigned int index = 0;
            for (typename std::vector<Rule*>::const_iterator ruleIt = rules.begin(); ruleIt != rules.end(); ++ruleIt)
            {
                Rule* rule = *ruleIt;
                AABB* area = rule->getArea();
                AABB* bounds = GetSweptVolumeFromLastKnownPosition(player);
                if (area->intersects(bounds->mMinX, bounds->mMinY, bounds->mMinZ, bounds->mMaxX, bounds->mMaxY, bounds->mMaxZ))
                {
                    (this->*callback)(player, index, rule);
                }
                ++index;
            }
        }
    }
}
template void CommonMasterGameMode::CheckIntersectsNamedAreas<CheckpointRuleDefinition>(const std::vector<boost::shared_ptr<Player> >&, const std::vector<CheckpointRuleDefinition*>&, void (CommonMasterGameMode::*)(const boost::shared_ptr<Player>&, unsigned int, CheckpointRuleDefinition*));
template void CommonMasterGameMode::CheckIntersectsNamedAreas<TargetAreaRuleDefinition>(const std::vector<boost::shared_ptr<Player> >&, const std::vector<TargetAreaRuleDefinition*>&, void (CommonMasterGameMode::*)(const boost::shared_ptr<Player>&, unsigned int, TargetAreaRuleDefinition*));
template void CommonMasterGameMode::CheckIntersectsNamedAreas<PowerupRuleDefinition>(const std::vector<boost::shared_ptr<Player> >&, const std::vector<PowerupRuleDefinition*>&, void (CommonMasterGameMode::*)(const boost::shared_ptr<Player>&, unsigned int, PowerupRuleDefinition*));
