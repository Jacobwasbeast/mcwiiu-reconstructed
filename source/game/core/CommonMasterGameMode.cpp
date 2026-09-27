#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include <cfloat>
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConditionConnective.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GameCondition.hpp"
#include "game/core/OnGameStartSpawnPositions.hpp"
#include "game/core/SpawnPositionSet.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/PowerupRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/PlayersEnteredVolumeCondition.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"
#include "game/core/StatsUID.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
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
    static Minecraft* minecraft = Minecraft::GetInstance();
    MinecraftServer* server = gMinecraftServer;
    if ((server == NULL || server->mMasterGameMode == NULL) && minecraft != NULL)
        return minecraft->GetClientMasterGameMode();
    return server->mMasterGameMode;
}

CommonMasterGameMode::CommonMasterGameMode()
{
    mMapGenerator = NULL;
    mNextLevelGenOptions = NULL;
    mCurrentLevelGenOptions = NULL;
    mGeneratorSeedScale = 1.0f;
    mGeneratorSeed = -1;
    mLocalizedStringTable = new StringTable();
    mEnglishStringTable = new StringTable();
    mDistancesCached = false;
    InitializeCriticalSection(&mGeneratorMutex);
}

CommonMasterGameMode::~CommonMasterGameMode()
{
    delete mLocalizedStringTable;
    delete mEnglishStringTable;
}

void CommonMasterGameMode::Setup(EMiniGameId miniGameId)
{
    if (miniGameId != eMiniGameId_Build)
        return;

    std::wstring archiveName(L"gameModeLanguages.loc");
    if (gApp.hasArchiveFile(archiveName, false))
    {
        delete mLocalizedStringTable;
        delete mEnglishStringTable;
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(archiveName, false);
        mLocalizedStringTable = new StringTable(data, false, false);
        mEnglishStringTable = new StringTable(data, false, false);
        mEnglishStringTable->SelectLanguage(L"en-EN");
        delete[] data.data;
    }
}

void CommonMasterGameMode::Tick()
{
    if (GeneratorTargetLevel() == NULL || GeneratorTargetLevel()->mIsClientSide)
        return;

    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL)
    {
        const unsigned int state = GetGameModeState();
        if (state > eGameModeState_SelectingGameRules && (state < eGameModeState_PostRound || state == eGameModeState_LoadingSubRound))
            mMapGenerator->ContinueGeneration();
    }
    LeaveCriticalSection(&mGeneratorMutex);
}

const MiniGameDef* CommonMasterGameMode::GetMiniGame()
{
    return Minecraft::GetInstance()->GetMiniGame();
}

void CommonMasterGameMode::SetupMiniGameInstance(const MiniGameDef& definition, int variant)
{
    Minecraft::GetInstance()->SetupMiniGameInstance(definition, 0);
    gMinecraftServer->setPvpAllowed(definition.AllowPvP());
    maybe<int> difficulty = definition.getDifficulty();
    if (difficulty.isSet())
        gApp.SetGameHostOption(eGameHostOption_Difficulty, difficulty.get());
}

void CommonMasterGameMode::OnExitedGame()
{
    mRoundInfo.reset();
}

EMiniGameId CommonMasterGameMode::GetNextGameMode() const
{
    if (mNextLevelGenOptions == NULL)
        return eMiniGameId_Lobby;
    return static_cast<EMiniGameId>(mNextLevelGenOptions->getRequiredGameRules()->getRuleType());
}

AABB* CommonMasterGameMode::GetTeamArea(Team* team) const
{
    LevelRuleset* rules = gApp.getLevelGenerationOptions()->getRequiredGameRules();
    std::vector<AABB*> areas;
    rules->getNamedAreas(1 << team->getTeamIndex(), &areas);
    if (areas.empty())
        return NULL;
    return areas.at(0);
}

Team* CommonMasterGameMode::GetTeamAtPosition(double x, double y, double z, double margin) const
{
    Level* level = Minecraft::GetInstance()->getLevel(0);
    std::vector<PlayerTeam*>* teams = level->GetScoreboard()->getPlayerTeams();
    for (unsigned int i = 0; i < teams->size(); ++i)
    {
        Team* team = teams->at(i);
        AABB* area = GetTeamArea(team);
        if (area == NULL)
            continue;

        AABB* bounds = area->grow(margin, margin, margin);
        if (bounds->mMinX <= x && x <= bounds->mMaxX
            && bounds->mMinY <= y && y <= bounds->mMaxY
            && bounds->mMinZ <= z && z <= bounds->mMaxZ)
        {
            return team;
        }
    }
    return NULL;
}

bool CommonMasterGameMode::HasFallenOffLayer(const double& previousY, const double& currentY)
{
    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL)
    {
        const unsigned int layerCount = mMapGenerator->GetNumberOfLayers();
        for (unsigned int layer = 0; layer < layerCount; ++layer)
        {
            float minimum;
            float maximum;
            mMapGenerator->GetLayerExtents(layer, &minimum, &maximum);
            minimum += 0.8f;
            if (minimum <= previousY && currentY < minimum)
            {
                LeaveCriticalSection(&mGeneratorMutex);
                return true;
            }
        }
    }
    LeaveCriticalSection(&mGeneratorMutex);
    return false;
}

bool CommonMasterGameMode::IsOnGeneratedLayer(const int& layer, const double& y)
{
    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL)
    {
        float minimum = 0.0f;
        float maximum = 0.0f;
        mMapGenerator->GetLayerExtents(layer, &minimum, &maximum);
        if (minimum <= y && y <= maximum + 1.0f)
        {
            LeaveCriticalSection(&mGeneratorMutex);
            return true;
        }
    }
    LeaveCriticalSection(&mGeneratorMutex);
    return false;
}

int CommonMasterGameMode::GetLayer(const double& y)
{
    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL)
    {
        const unsigned int layerCount = mMapGenerator->GetNumberOfLayers();
        for (unsigned int layer = 0; layer < layerCount; ++layer)
        {
            if (mMapGenerator->GetLayerBB(layer) != NULL
                && mMapGenerator->GetLayerBB(layer)->mMinY <= y
                && y < mMapGenerator->GetLayerBB(layer)->mMaxY)
            {
                LeaveCriticalSection(&mGeneratorMutex);
                return layer;
            }
        }
    }
    LeaveCriticalSection(&mGeneratorMutex);
    return -1;
}

unsigned int CommonMasterGameMode::GetLayerCount()
{
    if (mMapGenerator == NULL)
        return 0;
    return mMapGenerator->GetNumberOfLayers();
}

AABB* CommonMasterGameMode::GetLayerAABB(const int& layer)
{
    if (mMapGenerator == NULL)
        return NULL;
    return mMapGenerator->GetLayerBB(layer);
}

void CommonMasterGameMode::UpdateGeneratorSeed(long long seed, float scale)
{
    mGeneratorSeedScale = scale;
    mGeneratorSeed = seed;
}

void CommonMasterGameMode::RestartMapGenerator()
{
    EnterCriticalSection(&mGeneratorMutex);

    LevelRuleset* rules = gApp.getLevelGenerationOptions()->getRequiredGameRules();
    const long long seed = mGeneratorSeed;
    mGeneratorSeed = -1;
    Random random(seed);

    const EMiniGameId miniGameId = GetMiniGame()->GetId();
    if (miniGameId == eMiniGameId_Tumble)
    {
        LayerGenerationRuleDefinition* layerRule = static_cast<LayerGenerationRuleDefinition*>(
            rules->getRuleByType(ConsoleGameRules::eGameRuleType_LayerGeneration));
        AABB* area = rules->getNamedArea(L"LevelGeneration");
        delete mMapGenerator;

        Random layerRandom(seed);
        std::vector<LayerGenerator*>* layers =
            layerRule->CreateLayers(layerRandom, area, GetMiniGame()->GetLayerCount(), mGeneratorSeedScale);
        mMapGenerator = new TumbleGenerator(layerRandom, *layers);
        mMapGenerator->StartGeneration(GeneratorTargetLevel());
        delete layers;
    }
    else if (miniGameId == eMiniGameId_Build)
    {
        std::vector<AABB*> buildAreas;
        rules->getNamedAreas(0xff, &buildAreas);
        std::vector<AABB*> wallAreas;
        rules->getNamedAreas(0xff00, &wallAreas);
        delete mMapGenerator;

        mMapGenerator = new WallGenerator(random, &buildAreas, &wallAreas);
        mMapGenerator->StartGeneration(GeneratorTargetLevel());
    }

    LeaveCriticalSection(&mGeneratorMutex);
}

void CommonMasterGameMode::RemoveMapGenerator()
{
    EnterCriticalSection(&mGeneratorMutex);
    delete mMapGenerator;
    mMapGenerator = NULL;
    LeaveCriticalSection(&mGeneratorMutex);
}

void CommonMasterGameMode::CleanUpMapGenerator()
{
    if (GeneratorTargetLevel()->mIsClientSide)
        return;

    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL)
        mMapGenerator->CleanUp(GeneratorTargetLevel());
    LeaveCriticalSection(&mGeneratorMutex);
}

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
    return std::wstring(mEnglishStringTable->getString(id));
}

LevelGenerationOptions* CommonMasterGameMode::getRoundLevelGenOptions() const
{
    if (mNextLevelGenOptions != NULL && !mNextLevelGenOptions->isGameModeLobby())
        return mNextLevelGenOptions;
    if (mCurrentLevelGenOptions != NULL && !mCurrentLevelGenOptions->isGameModeLobby())
        return mCurrentLevelGenOptions;
    return gApp.getLevelGenerationOptions();
}

unsigned int CommonMasterGameMode::CheckpointIDtoSequenceID(int id) const
{
    if (id < 0 || static_cast<unsigned int>(id) >= mCheckpoints.size())
        return 0;
    return mCheckpoints.at(id)->getSequenceID();
}

void CommonMasterGameMode::setCheckpoints(std::vector<CheckpointRuleDefinition*> checkpoints)
{
    mCheckpoints = checkpoints;
    mFinalCheckpointSequenceID = 0;
    for (std::vector<CheckpointRuleDefinition*>::const_iterator it = mCheckpoints.begin(); it != mCheckpoints.end(); ++it)
    {
        const int sequenceID = (*it)->getSequenceID();
        if (sequenceID >= mFinalCheckpointSequenceID)
            mFinalCheckpointSequenceID = sequenceID;
    }
}

double CommonMasterGameMode::getTotalDistance()
{
    if (!mDistancesCached)
        CacheMapDistance();
    return mTotalDistance;
}

std::vector<double>* CommonMasterGameMode::getCheckpointDistances()
{
    if (!mDistancesCached)
        CacheMapDistance();
    return &mCheckpointDistances;
}

void CommonMasterGameMode::ResetMapDistances()
{
    mSecondaryTargetCounts[0] = 0;
    mSecondaryTargetCounts[1] = 0;
    mSecondaryTargetCounts[2] = 0;
    mCheckpointDistances.clear();
    mSequenceDistances.clear();
    mDistancesCached = false;
    mRequiredLaps = 0;
    mTotalDistance = 0.0;
}

void CommonMasterGameMode::CacheMapDistance()
{
    ResetMapDistances();

    std::vector<Vec3*> spawnPositions;
    LevelRuleset* ruleDefinitions = gApp.getGameRuleDefinitions();
    OnGameStartSpawnPositions* spawnRule = ruleDefinitions == NULL ? NULL :
        static_cast<OnGameStartSpawnPositions*>(ruleDefinitions->getRuleByType(ConsoleGameRules::eGameRuleType_OnGameStartSpawnPositions));
    if (spawnRule != NULL)
    {
        const bool centreSpawn = GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_CentreSpawn);
        GameRuleDefinition* spawnSet = spawnRule->getSpawnSet(!centreSpawn);
        if (spawnSet == NULL)
        {
            std::vector<GameRuleDefinition*> sets;
            spawnRule->getChildren(&sets);
            spawnSet = sets.front();
        }

        std::vector<GameRuleDefinition*> spawns;
        if (spawnSet != NULL)
        {
            spawnSet->getChildren(&spawns);
            for (std::vector<GameRuleDefinition*>::const_iterator it = spawns.begin(); it != spawns.end(); ++it)
            {
                UpdatePlayerRuleDefinition* spawn = static_cast<UpdatePlayerRuleDefinition*>(*it);
                spawnPositions.push_back(Vec3::newTemp(spawn->getX(), spawn->getY(), spawn->getZ()));
            }
        }
    }

    if (mCheckpoints.empty())
    {
        double distance = 0.0;
        for (std::vector<Vec3*>::const_iterator spawn = spawnPositions.begin(); spawn != spawnPositions.end(); ++spawn)
        {
            for (std::vector<AABB*>::const_iterator area = mPrimaryTargetAreas.begin(); area != mPrimaryTargetAreas.end(); ++area)
                distance = std::max(distance, (*spawn)->distanceTo(*area));
        }
        mSequenceDistances.push_back(distance);
        mCheckpointDistances.push_back(distance);
        mTotalDistance += distance;
    }
    else
    {
        double startDistance = 0.0;
        for (std::vector<CheckpointRuleDefinition*>::const_iterator checkpoint = mCheckpoints.begin(); checkpoint != mCheckpoints.end(); ++checkpoint)
        {
            if ((*checkpoint)->getSequenceID() != 0)
                continue;
            for (std::vector<Vec3*>::const_iterator spawn = spawnPositions.begin(); spawn != spawnPositions.end(); ++spawn)
                startDistance = std::max(startDistance, (*spawn)->distanceTo((*checkpoint)->getArea()));
        }
        mSequenceDistances.push_back(startDistance);
        mTotalDistance += startDistance;

        double sequenceDistance = 0.0;
        double checkpointDistance = 0.0;
        int sequenceID = 0;
        bool finished = false;
        do
        {
            std::vector<CheckpointRuleDefinition*> nextCheckpoints =
                gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpointsBySequenceID(sequenceID + 1);
            std::vector<CheckpointRuleDefinition*> currentCheckpoints =
                gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpointsBySequenceID(sequenceID);
            if (nextCheckpoints.empty())
            {
                finished = true;
                continue;
            }

            bool hasSpawnPoint = false;
            double distance = 0.0;
            for (std::vector<CheckpointRuleDefinition*>::const_iterator next = nextCheckpoints.begin(); next != nextCheckpoints.end(); ++next)
            {
                hasSpawnPoint = hasSpawnPoint || (*next)->hasSpawnPoint();
                for (std::vector<CheckpointRuleDefinition*>::const_iterator current = currentCheckpoints.begin(); current != currentCheckpoints.end(); ++current)
                    distance = std::max(distance, (*next)->getArea()->getCenter()->distanceTo((*current)->getArea()));
            }

            sequenceDistance += distance;
            checkpointDistance += distance;
            if (hasSpawnPoint)
            {
                mCheckpointDistances.push_back(checkpointDistance);
                checkpointDistance = 0.0;
            }
            mSequenceDistances.push_back(sequenceDistance);
            mTotalDistance += distance;
            sequenceDistance = 0.0;
            ++sequenceID;
        } while (!finished);

        std::vector<CheckpointRuleDefinition*> lastCheckpoints =
            gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpointsBySequenceID(sequenceID);
        double endDistance = 0.0;
        for (std::vector<CheckpointRuleDefinition*>::const_iterator checkpoint = lastCheckpoints.begin(); checkpoint != lastCheckpoints.end(); ++checkpoint)
        {
            for (std::vector<AABB*>::const_iterator area = mPrimaryTargetAreas.begin(); area != mPrimaryTargetAreas.end(); ++area)
                endDistance = std::max(endDistance, (*checkpoint)->getArea()->getCenter()->distanceTo(*area));
        }
        mSequenceDistances.push_back(endDistance);
        mTotalDistance += endDistance;
    }

    std::vector<TargetAreaRuleDefinition*> targetAreas;
    gApp.getLevelGenerationOptions()->getRequiredGameRules()->getTargetAreas(&targetAreas);
    for (std::vector<TargetAreaRuleDefinition*>::const_iterator target = targetAreas.begin(); target != targetAreas.end(); ++target)
    {
        if ((*target)->mPrimary)
            mRequiredLaps = (*target)->mLaps;
        else
            ++mSecondaryTargetCounts[(*target)->getSize()];
    }

    mDistancesCached = true;
}

void CommonMasterGameMode::CheckProgress(boost::shared_ptr<Player> player, const std::vector<CheckpointRuleDefinition*>* rules, const std::vector<AABB*>* areas)
{
    if (!GetMiniGame()->HasFeatureCheckpoints())
        return;

    int checkpointID;
    int lapsCompleted;
    if (player->GetType() == EntityType::GHOST_PLAYER)
    {
        boost::shared_ptr<GhostPlayer> ghost = boost::static_pointer_cast<GhostPlayer>(player);
        checkpointID = ghost->getLastCheckpointId();
        lapsCompleted = ghost->getLapsCompleted();
    }
    else
    {
        checkpointID = GetLastCheckpointID(player->getSUID());
        lapsCompleted = getScoreboard()->GetLapsCompleted(player->getSUID(), -1);
    }

    unsigned int sequenceID = 0;
    if (checkpointID != -1)
        sequenceID = rules->at(checkpointID)->getSequenceID() + 1;

    std::vector<CheckpointRuleDefinition*> nextCheckpoints =
        gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpointsBySequenceID(sequenceID);
    if (nextCheckpoints.empty() && lapsCompleted < mRequiredLaps)
    {
        OnLapCompleted(player);
        nextCheckpoints = gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpointsBySequenceID(0);
    }

    double nearestDistance = DBL_MAX;
    if (nextCheckpoints.empty())
    {
        for (std::vector<AABB*>::const_iterator area = areas->begin(); area != areas->end(); ++area)
            nearestDistance = std::min(nearestDistance, player->getPos(1.0f)->distanceTo(*area));
    }
    else
    {
        for (std::vector<CheckpointRuleDefinition*>::const_iterator checkpoint = nextCheckpoints.begin(); checkpoint != nextCheckpoints.end(); ++checkpoint)
            nearestDistance = std::min(nearestDistance, player->getPos(1.0f)->distanceTo((*checkpoint)->getArea()));
    }

    const double sequenceProgress = mSequenceDistances.at(sequenceID) - nearestDistance;
    double distanceTravelled = sequenceProgress;
    for (unsigned int i = 0; i < sequenceID; ++i)
        distanceTravelled += mSequenceDistances.at(i);

    const double progress = (distanceTravelled / mTotalDistance) * 100.0;
    const double speed = Vec3::newTemp(player->mXo, player->mYo, player->mZo)->distanceTo(
        Vec3::newTemp(player->mX, player->mY, player->mZ)) * 20.0;

    GameMode* gameMode = player->GetGameMode();
    if (gameMode != NULL)
        gameMode->RecordProgress(sequenceProgress, nearestDistance, progress, speed);

    OnProgressMade(player, progress);
}

void CommonMasterGameMode::CheckProgress(const std::vector<boost::shared_ptr<Player> >& players)
{
    if (!GetMiniGame()->HasFeatureCheckpoints())
        return;
    if (mCheckpoints.empty() && mPrimaryTargetAreas.empty())
        return;

    if (!mDistancesCached)
        CacheMapDistance();

    for (std::vector<boost::shared_ptr<Player> >::const_iterator it = players.begin(); it != players.end(); ++it)
        CheckProgress(*it, &mCheckpoints, &mPrimaryTargetAreas);
}

void CommonMasterGameMode::OnLapCompleted(const boost::shared_ptr<Player>& player)
{
    GameMode* gameMode = player->GetGameMode();
    if (gameMode != NULL)
    {
        gameMode->RecordLastCheckpoint(-1, -1, false, false, true);
        gameMode->RecordLapCompleted();
    }
}

void CommonMasterGameMode::CheckCheckpoints(const std::vector<boost::shared_ptr<Player> >& players)
{
    CheckIntersectsNamedAreas(players, mCheckpoints, &CommonMasterGameMode::OnCheckpointEnter);
}

void CommonMasterGameMode::CheckTargets(const std::vector<boost::shared_ptr<Player> >& players)
{
    std::vector<TargetAreaRuleDefinition*> targetAreas;
    gApp.getLevelGenerationOptions()->getRequiredGameRules()->getTargetAreas(&targetAreas);
    CheckIntersectsNamedAreas(players, targetAreas, &CommonMasterGameMode::OnTargetEnter);
}

void CommonMasterGameMode::CheckPowerups(const std::vector<boost::shared_ptr<Player> >& players)
{
    if (!GetMiniGame()->ArePowerupsActive())
        return;

    std::vector<PowerupRuleDefinition*> powerups;
    gApp.getLevelGenerationOptions()->getRequiredGameRules()->getPowerups(&powerups);
    CheckIntersectsNamedAreas(players, powerups, &CommonMasterGameMode::OnPowerupEnter);
}

void CommonMasterGameMode::OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint, CheckpointRuleDefinition* rule)
{
    GameMode* gameMode = player->GetGameMode();
    if (gameMode != NULL)
        gameMode->RecordLastCheckpoint(checkpoint, rule->getSequenceID(), rule->hasSpawnPoint(), rule->hasSpawnPoint(), false);
}

void CommonMasterGameMode::OnTargetEnter(const boost::shared_ptr<Player>& player, unsigned int target, TargetAreaRuleDefinition* rule)
{
    const MiniGameDef* miniGame = GetMiniGame();
    bool usingPrimaryTargetAreas = false;

    ConditionConnective* endConditions = dynamic_cast<ConditionConnective*>(miniGame->GetEndConditions());
    if (endConditions != NULL)
    {
        const std::vector<GameCondition*>& conditions = endConditions->GetConditions();
        for (unsigned int i = 0; i < conditions.size(); ++i)
        {
            PlayersEnteredVolumeCondition* volumeCondition = dynamic_cast<PlayersEnteredVolumeCondition*>(conditions.at(i));
            if (volumeCondition != NULL && volumeCondition->IsUsingPrimaryTargetAreas())
            {
                usingPrimaryTargetAreas = true;
                break;
            }
        }
    }

    if (getScoreboard()->GetEndZoneEntered(player->getSUID(), -1))
        return;
    if (player->isSpectator())
        return;

    if (!rule->mPrimary && usingPrimaryTargetAreas)
    {
        if (miniGame->ArePointsActive())
            OnSecondaryTargetEnter(player, rule);
    }
    else if (rule->mLaps <= getScoreboard()->GetLapsCompleted(player->getSUID(), -1)
             && getScoreboard()->GetLastCheckpointSequenceID(player->getSUID(), -1) == mFinalCheckpointSequenceID)
    {
        OnPrimaryTargetEnter(player, rule);
    }
}

void CommonMasterGameMode::OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int powerup, PowerupRuleDefinition* rule)
{
    rule->GiveRandomPowerup(player);
}

void CommonMasterGameMode::OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
}

void CommonMasterGameMode::OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
}

void CommonMasterGameMode::OnProgressMade(const boost::shared_ptr<Player>& player, double progress)
{
}

void CommonMasterGameMode::setChosenThemeWordId(int id)
{
    mChosenThemeWordId = id;
}

template<class Rule> void CommonMasterGameMode::CheckIntersectsNamedAreas(const std::vector<boost::shared_ptr<Player> >& players, std::vector<Rule*>& rules, void (CommonMasterGameMode::*callback)(const boost::shared_ptr<Player>&, unsigned int, Rule*))
{
    if (rules.empty())
        return;

    for (typename std::vector<boost::shared_ptr<Player> >::const_iterator player = players.begin(); player != players.end(); ++player)
    {
        if ((*player)->GetType() != EntityType::GHOST_PLAYER && !(*player)->isAlive())
            continue;

        unsigned int index = 0;
        for (typename std::vector<Rule*>::iterator rule = rules.begin(); rule != rules.end(); ++rule, ++index)
        {
            AABB* area = (*rule)->getArea();
            AABB* bounds = GetSweptVolumeFromLastKnownPosition(*player);
            if (area->intersects(bounds->mMinX, bounds->mMinY, bounds->mMinZ, bounds->mMaxX, bounds->mMaxY, bounds->mMaxZ))
                (this->*callback)(*player, index, *rule);
        }
    }
}

