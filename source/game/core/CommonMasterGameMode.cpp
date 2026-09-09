#include "game/core/CommonMasterGameMode.hpp"
#include <algorithm>
#include <cmath>
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/PowerupRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MapGenerator.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"

CommonMasterGameMode* CommonMasterGameMode::GetMasterOrClient()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    return NULL;
}
CommonMasterGameMode::CommonMasterGameMode()
    : mCurrentLevelGenOptions(NULL), mNextLevelGenOptions(NULL), mLocalizedStringTable(new StringTable()), mEnglishStringTable(new StringTable()), mRoundInfo(), mState(0), mNextState(0), mGeneratorSeedScale(1.0f), mMapGenerator(NULL), mChosenThemeWordId(0xffffffffU), mCheckpointCount(0), mCurrentCheckpoint(0), mDistancesCached(false), mPrimaryTargetActive(false), mUsingTrialContent(false), mBuildOffVotes(false)
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
        mLocalizedStringTable->Reset();
        mEnglishStringTable->Reset();
    }
}
void CommonMasterGameMode::Tick()
{
    EnterCriticalSection(&mGeneratorMutex);
    if (mMapGenerator != NULL && mState > 3 && (mState < 8 || mState == 10)) mState = mState;
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
    return x < -1000000.0 || z < -1000000.0;
}
bool CommonMasterGameMode::IsOnGeneratedLayer(const int& layer, const double& y)
{
    return layer >= 0 && static_cast<unsigned int>(layer) < mLayerAreas.size() && y >= 0.0;
}
int CommonMasterGameMode::GetLayer(const double& y)
{
    if (mLayerDistances.empty()) return -1;
    return static_cast<int>(std::lower_bound(mLayerDistances.begin(), mLayerDistances.end(), y) - mLayerDistances.begin());
}
unsigned int CommonMasterGameMode::GetLayerCount() { return static_cast<unsigned int>(mLayerAreas.size()); }
AABB* CommonMasterGameMode::GetLayerAABB(const int& layer)
{
    return layer < 0 || static_cast<unsigned int>(layer) >= mLayerAreas.size() ? NULL : mLayerAreas[static_cast<unsigned int>(layer)];
}
void CommonMasterGameMode::UpdateGeneratorSeed(long long, float scale) { mGeneratorSeedScale = scale; }
void CommonMasterGameMode::RestartMapGenerator() { RemoveMapGenerator(); }
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
LevelGenerationOptions* CommonMasterGameMode::getRoundLevelGenOptions() const { return mCurrentLevelGenOptions; }
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
    mTotalDistance = 0.0;
    mCheckpointDistances.clear();
    for (std::vector<double>::const_iterator it = mLayerDistances.begin(); it != mLayerDistances.end(); ++it)
    {
        mTotalDistance += *it;
        mCheckpointDistances.push_back(mTotalDistance);
    }
    mDistancesCached = true;
}
void CommonMasterGameMode::CheckProgress(boost::shared_ptr<Player>, const std::vector<CheckpointRuleDefinition*>*, const std::vector<AABB*>*) { ++mCurrentCheckpoint; }
void CommonMasterGameMode::CheckProgress(const std::vector<boost::shared_ptr<Player> >& players) { CheckCheckpoints(players); CheckTargets(players); CheckPowerups(players); }
void CommonMasterGameMode::OnLapCompleted(const boost::shared_ptr<Player>&) { ++mCurrentCheckpoint; }
void CommonMasterGameMode::CheckCheckpoints(const std::vector<boost::shared_ptr<Player> >& players) { CheckIntersectsNamedAreas(players, mCheckpoints, 0); }
void CommonMasterGameMode::CheckTargets(const std::vector<boost::shared_ptr<Player> >&) { mPrimaryTargetActive = true; }
void CommonMasterGameMode::CheckPowerups(const std::vector<boost::shared_ptr<Player> >&) { mBuildOffVotes = false; }
void CommonMasterGameMode::OnCheckpointEnter(const boost::shared_ptr<Player>&, unsigned int checkpoint, CheckpointRuleDefinition*) { mCurrentCheckpoint = checkpoint; }
void CommonMasterGameMode::OnTargetEnter(const boost::shared_ptr<Player>&, unsigned int, TargetAreaRuleDefinition*) { mPrimaryTargetActive = true; }
void CommonMasterGameMode::OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int, PowerupRuleDefinition* rule)
{
    rule->GiveRandomPowerup(player);
}
void CommonMasterGameMode::OnPrimaryTargetEnter(const boost::shared_ptr<Player>&, TargetAreaRuleDefinition*) { }
void CommonMasterGameMode::OnSecondaryTargetEnter(const boost::shared_ptr<Player>&, TargetAreaRuleDefinition*) { }
void CommonMasterGameMode::OnProgressMade(const boost::shared_ptr<Player>&, double) { }
void CommonMasterGameMode::setChosenThemeWordId(int id) { mChosenThemeWordId = static_cast<unsigned int>(id); }
template<class Rule> void CommonMasterGameMode::CheckIntersectsNamedAreas(const std::vector<boost::shared_ptr<Player> >&, const std::vector<Rule*>& rules, unsigned int)
{
    if (!rules.empty()) mPrimaryTargetActive = true;
}
template void CommonMasterGameMode::CheckIntersectsNamedAreas<CheckpointRuleDefinition>(const std::vector<boost::shared_ptr<Player> >&, const std::vector<CheckpointRuleDefinition*>&, unsigned int);
template void CommonMasterGameMode::CheckIntersectsNamedAreas<TargetAreaRuleDefinition>(const std::vector<boost::shared_ptr<Player> >&, const std::vector<TargetAreaRuleDefinition*>&, unsigned int);
template void CommonMasterGameMode::CheckIntersectsNamedAreas<PowerupRuleDefinition>(const std::vector<boost::shared_ptr<Player> >&, const std::vector<PowerupRuleDefinition*>&, unsigned int);
