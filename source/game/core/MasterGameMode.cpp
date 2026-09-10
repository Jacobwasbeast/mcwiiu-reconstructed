#include "game/core/MasterGameMode.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/System.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/util/Vec3.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/Minecraft.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/block/BlockRemover.hpp"
#include <algorithm>
#include <cmath>
#include <deque>
#include "game/client/render/TexturePackRepository.hpp"

extern void instructionSynchronize();

typedef boost::unordered_map<MasterGameMode::ETimerID, MasterGameMode::_TimerInfo> TimerMap;

MasterGameMode::MasterGameMode(std::vector<LevelRules::LevelRulesetGroup*>* playlist)
    : mRoundStartTime(0), mRoundTimeOffset(0), mPausedRoundTime(0), mShowdownTime(0), mPostGameTime(0), mTimers(), mNextLevelGenerationOptions(0), mGameStats(), mInternalState(eInternalGameModeState_Lobby), mPreviousState(eInternalGameModeState_Lobby), mPlaylist(), mCurrentPlaylistEntry(0), mCurrentRuleIndex(0), mReadyPlayers(0), mRequiredPlayers(0), mMinimumPlayers(0), mMaximumPlayers(8), mChestRefills(), mKillboxes(), mKillboxAreas(), mActiveChunkBounds(), mRandom(), mLockPlayerPositions(false), mRoundStarted(false), mRoundEnding(false), mShowdownStarted(false), mPlaylistDirty(false), mTutorialDirty(false), mPlayersPrepared(false), mReturningToLobby(false), mGeneratorSpawn(0), mSpawnLayerIndex(0), mChunkHistory(0), mRoundWinner(-1), mActiveTeamCount(0), mShowdownReduction(0), mChosenThemeWordId(0), mTutorialData(), mRoundLevel(0), mBlockRemover(0), mLastKnownPositionPlayers(), mScoreboard(0)
{
    for (int index = 0; index < 8; ++index) { mPlayerReady[index] = false; mLastKnownPositions[index] = 0; }
    if (playlist != 0) mPlaylist = *playlist;
    ResetRoundTime();
}

MasterGameMode::~MasterGameMode()
{
    KillTimers();
}

std::vector<boost::shared_ptr<ServerPlayer> > MasterGameMode::GetPlayers(bool includeSpectators, bool includeDead)
{
    std::vector<boost::shared_ptr<ServerPlayer> > players;
    players.reserve(mMaximumPlayers);
    return players;
}

int MasterGameMode::GetPlayerCount(bool includeSpectators, bool includeDead)
{
    return static_cast<int>(GetPlayers(includeSpectators, includeDead).size());
}

unsigned long long MasterGameMode::GetCurrentTimeMillis()
{
    return System::processTimeInMilliSecs();
}

bool MasterGameMode::HasTimer(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    return entry != mTimers.end() && entry->second.active;
}

void MasterGameMode::KillTimer(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry != mTimers.end())
        entry->second.active = false;
}

void MasterGameMode::ResetRoundTime()
{
    mRoundStartTime = GetCurrentTimeMillis();
    mRoundTimeOffset = 0;
    mPausedRoundTime = 0;
    mShowdownTime = 0;
    mPostGameTime = 0;
    KillTimer(eTimerID_RoundLimit);
}

void MasterGameMode::StaticCtor()
{
    System::processTimeInMilliSecs();
}

void MasterGameMode::GeneratePlaylistSyncInfo()
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetReadyStates(bool allPlayers)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetLastKnownPosition(int playerIndex)
{
    mPlaylistDirty = true;
}

int MasterGameMode::GetMinimumPlayersToStart()
{
    return mMinimumPlayers;
}

MasterGameMode::EInternalGameModeState MasterGameMode::GetState()
{
    return static_cast<EInternalGameModeState>(mInternalState);
}

void MasterGameMode::RestartTimer(ETimerID timer, int duration)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry != mTimers.end())
    {
        entry->second.startTime = GetCurrentTimeMillis();
        entry->second.duration = duration;
        entry->second.endTime = duration > 0 ? entry->second.startTime + duration : -1;
        entry->second.active = true;
    }
}

void MasterGameMode::TickTimers()
{
    for (TimerMap::iterator entry = mTimers.begin(); entry != mTimers.end(); ++entry)
    {
        _TimerInfo& timer = entry->second;
        if (timer.active && timer.endTime >= 0 &&
            GetCurrentTimeMillis() >= static_cast<unsigned long long>(timer.endTime))
        {
            timer.active = false;
            if (timer.callback != 0)
                timer.callback(entry->first, this, timer.callbackData);
        }
    }
}

void MasterGameMode::SyncNextLevelGen()
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnGameModeChange(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::CheckTutorialUpdate()
{
    mPlaylistDirty = true;
}

bool MasterGameMode::IsInKillBox(boost::shared_ptr<Entity> entity)
{
    return mRoundStarted || mRoundEnding;
}

bool MasterGameMode::AreLevelChunksLoaded()
{
    return mRoundStarted || mRoundEnding;
}

void MasterGameMode::ChooseNextGameRules(bool advancePlaylist)
{
    LevelGenerationOptions* selectedOptions = 0;
    LevelRules::LevelRulesetGroup* selectedGroup = 0;
    int totalVotes = 0;

    Voteable* voteable = mVoteables->getVoteable(0);
    int winningVote = voteable->getWinningVote(totalVotes);
    if (winningVote != -1)
    {
        for (std::vector<LevelRules::LevelRulesetGroup*>::iterator playlistIt = mPlaylist.begin();
             playlistIt != mPlaylist.end(); ++playlistIt)
        {
            LevelRules::LevelRulesetGroup* group = *playlistIt;
            if (static_cast<short>(LevelGenerationOptions::GetMapSizeFromUniqueId(winningVote)) == -1)
            {
                if (!group->rulesets.empty() &&
                    group->rulesets.front()->getThemeId() ==
                        LevelGenerationOptions::GetThemeFromUniqueId(winningVote))
                {
                    selectedGroup = group;
                    break;
                }
            }
            else if (!group->rulesets.empty())
            {
                for (std::vector<LevelRuleset*>::iterator rulesetIt = group->rulesets.begin();
                     rulesetIt != group->rulesets.end(); ++rulesetIt)
                {
                    LevelGenerationOptions* options = (*rulesetIt)->getGenerationOptions();
                    if (options->getUniqueId() == static_cast<unsigned int>(winningVote))
                    {
                        selectedOptions = options;
                        selectedGroup = group;
                        break;
                    }
                }
                if (selectedGroup != 0)
                    break;
            }
        }

        if (selectedGroup != 0)
        {
            std::deque<LevelRules::LevelRulesetGroup*>::iterator found =
                std::find(mRuleGroups.begin(), mRuleGroups.end(), selectedGroup);
            const int targetIndex = mCurrentPlaylistEntry + mCurrentRuleIndex;
            if (found != mRuleGroups.end())
            {
                const int foundIndex = static_cast<int>(found - mRuleGroups.begin());
                if (foundIndex != targetIndex)
                {
                    mShuffledRuleGroups.push_back(selectedGroup);
                    mRuleGroups.erase(found);
                }
            }
        }
    }

    if (selectedGroup == 0)
    {
        selectedGroup = mRuleGroups.front();
        if (!advancePlaylist)
        {
            mShuffledRuleGroups.push_back(mRuleGroups.front());
            mRuleGroups.pop_front();
        }
    }

    if (mCurrentRuleIndex == 0)
    {
        mRuleGroups.insert(mRuleGroups.begin() + mCurrentPlaylistEntry,
                           mShuffledRuleGroups.begin(), mShuffledRuleGroups.end());
        mShuffledRuleGroups.clear();
    }

    if (selectedOptions != 0)
    {
        mCurrentLevelGenOptions = selectedOptions;
        SyncNextLevelGen();
        return;
    }

    int mapSizeIndex = 2;
    const MiniGameDef* miniGame = GetMiniGame();
    unsigned int mapSizeOverride = miniGame->GetMapSizeOverride();
    if (mapSizeOverride == 0)
    {
        unsigned int playerCount = GetPlayerCount(false, false);
        if (playerCount < 9)
            mapSizeIndex = 1;
        playerCount = GetPlayerCount(false, false);
        miniGame = GetMiniGame();
        if (miniGame->GetMaxPlayersForSmallMaps() >= playerCount)
            mapSizeIndex = 0;
    }
    else
    {
        mapSizeIndex = static_cast<int>(mapSizeOverride) - 1;
    }

    std::vector<LevelRuleset*>::iterator rulesetIt = selectedGroup->rulesets.begin();
    if (rulesetIt == selectedGroup->rulesets.end())
        goto select_fallback;

    while (true)
    {
        LevelGenerationOptions* options = (*rulesetIt)->getGenerationOptions();
        if (options == 0)
            goto next_ruleset;
        if (options->getMapSize() != static_cast<unsigned int>(mapSizeIndex))
            goto next_ruleset;
        options = (*rulesetIt)->getGenerationOptions();
        if (options == 0)
            goto select_fallback;
        mCurrentLevelGenOptions = options;
        SyncNextLevelGen();
        return;

    next_ruleset:
        ++rulesetIt;
        if (rulesetIt == selectedGroup->rulesets.end())
            goto select_fallback;
    }

select_fallback:
    do
    {
        while (mapSizeIndex != 2)
        {
            if (mapSizeIndex != 1)
            {
                selectedOptions = selectedGroup->rulesets.front()->getGenerationOptions();
                if (selectedOptions == 0)
                    return;
                mCurrentLevelGenOptions = selectedOptions;
                SyncNextLevelGen();
                return;
            }
            rulesetIt = selectedGroup->rulesets.begin();
            mapSizeIndex = 0;
            if (rulesetIt != selectedGroup->rulesets.end())
                goto select_search;
        }
        rulesetIt = selectedGroup->rulesets.begin();
        mapSizeIndex = 1;
    }
    while (rulesetIt == selectedGroup->rulesets.end());

select_search:
    while (true)
    {
        selectedOptions = (*rulesetIt)->getGenerationOptions();
        if (selectedOptions == 0)
            goto select_next_ruleset;
        if (selectedOptions->getMapSize() != static_cast<unsigned int>(mapSizeIndex))
            goto select_next_ruleset;
        selectedOptions = (*rulesetIt)->getGenerationOptions();
        if (selectedOptions == 0)
            goto select_fallback;
        mCurrentLevelGenOptions = selectedOptions;
        SyncNextLevelGen();
        return;

    select_next_ruleset:
        ++rulesetIt;
        if (rulesetIt == selectedGroup->rulesets.end())
            goto select_fallback;
    }
}

void MasterGameMode::UpdateClientState(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetState(EInternalGameModeState state)
{
    mPreviousState = mInternalState;
    mInternalState = state;
    mRoundStarted = state == eInternalGameModeState_RoundPlaying;
    mRoundEnding = state == eInternalGameModeState_RoundEnding;
}

void MasterGameMode::sendTutorialUpdate(eTutorial_GameMode tutorial)
{
    mPlaylistDirty = true;
}

void MasterGameMode::BroadcastChat(int message, std::vector<int>& arguments)
{
    mPlaylistDirty = true;
}

void MasterGameMode::CompleteTimer(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry != mTimers.end())
    {
        entry->second.active = false;
        if (entry->second.callback != 0)
            entry->second.callback(timer, this, entry->second.callbackData);
    }
}

void MasterGameMode::CheckReadyStates()
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnGameChange(MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->ChooseNextGameRules(context != 0);
}

void MasterGameMode::AddTimer(ETimerID timer, TimerCallback callback, void* context, int duration, int announceEvery, int changeAnnounceAfter, int finalAnnounceEvery)
{
    _TimerInfo& info = mTimers[timer];
    info.startTime = GetCurrentTimeMillis();
    info.duration = duration;
    info.endTime = duration > 0 ? info.startTime + duration : -1;
    info.announceEvery = announceEvery;
    info.changeAnnounceAfter = changeAnnounceAfter;
    info.finalAnnounceEvery = finalAnnounceEvery;
    info.nextAnnouncement = announceEvery < 0 ? info.endTime : info.startTime + announceEvery;
    info.active = true;
    info.callbackData = context;
    info.callback = callback;
}

void MasterGameMode::AddTimer(ETimerID timer, TimerCallback callback, void* context, MiniGameDef::TimerSpec specification)
{
    AddTimer(timer, callback, context, specification.TotalDurationMs(),
             specification.AnnounceEveryMs(false),
             specification.ChangeAnnounceAfterMs(),
             specification.AnnounceEveryMs(true));
}

int MasterGameMode::GetTimerRemainingTime(ETimerID timer)
{
    TimerMap::iterator entry = mTimers.find(timer);
    if (entry == mTimers.end() || !entry->second.active)
        return 0;
    if (entry->second.endTime < 0)
        return 0x7fffffff;
    return static_cast<int>(entry->second.endTime - GetCurrentTimeMillis());
}

void MasterGameMode::TimerCountdown(ETimerID timer, CountdownInfo* countdown)
{
    int remaining = GetTimerRemainingTime(timer);
    if (remaining > 0)
    {
        std::vector<int> arguments = countdown->arguments;
        arguments.insert(arguments.begin(), (remaining + 999) / 1000);
        BroadcastChat(countdown->message, arguments);
        if (countdown->tickCallback != 0)
            countdown->tickCallback(this, countdown->callbackData);
    }
    else if (countdown->completeCallback != 0)
    {
        countdown->completeCallback(this, countdown->callbackData);
    }
}

void MasterGameMode::OnTimerCountdown(ETimerID timer, MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->TimerCountdown(timer, static_cast<CountdownInfo*>(context));
}

void MasterGameMode::PopulateChests()
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == 0)
        return;

    GameRuleDefinition* definition = ruleset->getRuleByType(
        static_cast<ConsoleGameRules::EGameRuleType>(30));
    if (definition == 0)
        return;

    ServerLevel* level = gMinecraftServer->getLevel(0);

    std::vector<GameRuleDefinition*> children;
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(6));
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        (*it)->evaluateRule(level, ruleset, *miniGame);
    }

    children.clear();
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(14));
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        (*it)->evaluateRule(level, ruleset, *miniGame);
    }

    children.clear();
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(32));
    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        (*it)->evaluateRule(level, ruleset, *miniGame);
    }
}

int MasterGameMode::ChooseItemSet(const MiniGameDef& definition, bool central)
{
    return mInternalState;
}

int MasterGameMode::GetTeamCount()
{
    return mActiveTeamCount;
}

void MasterGameMode::SetupTeams()
{
    mPlaylistDirty = true;
}

void MasterGameMode::SelectNewGameRules()
{
    mPlaylistDirty = true;
}

void MasterGameMode::GetKillBoxes(LevelGenerationOptions* options, std::vector<KillboxRuleDefinition*>& killboxes)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetPlayers()
{
    mPlaylistDirty = true;
}

bool MasterGameMode::IsChunkWithinActiveBounds(void* context, int chunkX, int chunkZ)
{
    MasterGameMode* gameMode = static_cast<MasterGameMode*>(context);
    return gameMode != 0 && gameMode->IsChunkWithinLobbyRegionOfRoundLevel(chunkX, chunkZ);
}

void MasterGameMode::GenerateChestRefillInfo()
{
    mPlaylistDirty = true;
}

bool MasterGameMode::LoadNewGameRules()
{
    mPlaylistDirty = false;
    return !mPlaylist.empty();
}

void MasterGameMode::SetLockPlayerPositions(bool locked)
{
    mLockPlayerPositions = locked;
}

void MasterGameMode::UpdateGeneratorSeed()
{
    Random random;
    const long long seed = random.nextLong();

    int scaledPlayerCount = GetPlayerCount(false, false);
    if (scaledPlayerCount > 16)
        scaledPlayerCount = 16;
    if (scaledPlayerCount < 4)
        scaledPlayerCount = 4;

    const float scaleRatio =
        sqrtf(static_cast<float>(static_cast<double>(scaledPlayerCount))) / sqrtf(16.0f);
    const MiniGameDef* miniGame = GetMiniGame();
    const float scale = static_cast<float>(miniGame->GetLayerScale(scaleRatio));

    CommonMasterGameMode::UpdateGeneratorSeed(seed, scale);
    instructionSynchronize();
}

int MasterGameMode::GeneratorSpawn() const
{
    return mGeneratorSpawn;
}

boost::shared_ptr<ServerPlayer> MasterGameMode::GetRoundWinner()
{
    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(false, false);
    return players.empty() ? boost::shared_ptr<ServerPlayer>() : players[0];
}

int MasterGameMode::GetSpawnLayerIndex() const
{
    return mSpawnLayerIndex;
}

void MasterGameMode::PlacePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool atStart, bool includeSpectators)
{
    mPlaylistDirty = true;
}

void MasterGameMode::PositionPlayersAtStartingPosition()
{
    mPlaylistDirty = true;
}

void MasterGameMode::RemoveAllEntities(bool keepPlayers)
{
    mPlaylistDirty = true;
}

void MasterGameMode::AddPlayersToTeams()
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnWaitingToStartCountdown(ETimerID timer, MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->CompleteTimer(timer);
}

bool MasterGameMode::CanRoundStart()
{
    return mReadyPlayers >= mRequiredPlayers &&
           GetPlayerCount(false, false) >= mMinimumPlayers;
}

void MasterGameMode::SetGameMode(const GameType* gameType)
{
    mPlayersPrepared = gameType != 0;
}

void MasterGameMode::ResetPlayersFlyingState(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetPlayerHealths(std::vector<boost::shared_ptr<ServerPlayer> >* players, float health)
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetPlayerAbilities(boost::shared_ptr<ServerPlayer> player, const Abilities* abilities)
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetPlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players, const Abilities* abilities)
{
    mPlaylistDirty = true;
}

void MasterGameMode::UpdatePlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mPlaylistDirty = true;
}

bool MasterGameMode::ShouldRemoveArmourPiece(int slot)
{
    return slot >= 0 && slot < 4;
}

void MasterGameMode::EmptyPlayerInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool includeArmour)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetPlayersEquipmentAndInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mPlaylistDirty = true;
}

void MasterGameMode::PreparePlayersForGamemode(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetGameRules()
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetPlayerInvulnerable(boost::shared_ptr<ServerPlayer> player, bool invulnerable)
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetPlayersInvulnerable(bool invulnerable)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnRefillChestTimer(MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->PopulateChests();
}

void MasterGameMode::OnGracePeriodEnd(MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->SetPlayersInvulnerable(false);
}

bool MasterGameMode::StartTimer(ETimerID timer, CountdownInfo* countdown, int duration)
{
    if (countdown == 0 || duration < 0)
        return false;
    AddTimer(timer, &MasterGameMode::OnTimerCountdown, countdown,
             duration, -1, -1, -1);
    return true;
}

void MasterGameMode::StartTimer(ETimerID timer, CountdownInfo* countdown)
{
    if (countdown != 0)
        AddTimer(timer, &MasterGameMode::OnTimerCountdown, countdown,
                 countdown->arguments.empty() ? 0 : countdown->arguments[0],
                 -1, -1, -1);
}

void MasterGameMode::OnGameStart(MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->SetState(eInternalGameModeState_RoundPlaying);
}

void MasterGameMode::SetLastKnownPosition(boost::shared_ptr<Player> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::UpdateClientsCameras(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::Tick_Round_Playing()
{
    mPlaylistDirty = true;
}

bool MasterGameMode::CanRoundEnd()
{
    return mRoundEnding || GetRoundTime() >= mShowdownTime;
}

bool MasterGameMode::CanStartShowdown()
{
    return !mShowdownStarted && GetShowdownTime() > 0;
}

std::vector<boost::shared_ptr<ServerPlayer> > MasterGameMode::GetGlidePlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> > players;
    players.reserve(mMaximumPlayers);
    return players;
}

int MasterGameMode::GetShowdownTime()
{
    return mShowdownTime - mShowdownReduction;
}

void MasterGameMode::StartRoundEndTimer()
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnShowdownStart()
{
    mPlaylistDirty = true;
}

std::vector<PlayerTeam*> MasterGameMode::GetActiveTeams()
{
    std::vector<PlayerTeam*> teams;
    teams.reserve(mActiveTeamCount);
    return teams;
}

bool MasterGameMode::CanEndPostGame()
{
    return mPostGameTime <= 0;
}

bool MasterGameMode::ShouldPlayAnotherRound()
{
    return !mReturningToLobby &&
           mCurrentPlaylistEntry + 1 < static_cast<int>(mPlaylist.size());
}

void MasterGameMode::OnGameEnd(MasterGameMode* gameMode, void* context)
{
    if (gameMode != 0)
        gameMode->SetState(eInternalGameModeState_PostGame);
}

void MasterGameMode::ReturnPlayersToLobby()
{
    mPlaylistDirty = true;
}

void MasterGameMode::Tick()
{
    CommonMasterGameMode::Tick();
    TickTimers();
    if (mTutorialDirty)
        CheckTutorialUpdate();
    if (mRoundLevel != 0 && mRoundLevel->getServer() != 0 &&
        mRoundLevel->getServer()->getPlayers() != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players =
            mRoundLevel->getServer()->getPlayers()->getPlayers();
        if (players != 0)
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
                 it != players->end(); ++it)
                if (*it && IsInKillBox(*it))
                    (*it)->kill();
    }
    if (mBlockRemover != 0)
    {
        if (!mBlockRemover->Finished())
            mBlockRemover->Tick();
        else
        {
            delete mBlockRemover;
            mBlockRemover = 0;
        }
    }
    if (mInternalState == 0 && AreLevelChunksLoaded())
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft != 0 && minecraft->mTexturePackRepository != 0)
            minecraft->mTexturePackRepository->getSelected();
        mRuleGroups.clear();
        mShuffledRuleGroups.clear();
        mRuleGroups.insert(mRuleGroups.end(), mPlaylist.begin(), mPlaylist.end());
        mShuffledRuleGroups.insert(mShuffledRuleGroups.end(), mRuleGroups.begin(), mRuleGroups.end());
        std::random_shuffle(mShuffledRuleGroups.begin(), mShuffledRuleGroups.end());
        ChooseNextGameRules(true);
        SetState(static_cast<EInternalGameModeState>(2));
        sendTutorialUpdate(static_cast<eTutorial_GameMode>(0));
    }
}

void MasterGameMode::SetupServerSettings(Settings* settings)
{
    mPlaylistDirty = true;
}

const GameType* MasterGameMode::InitialGameType(boost::shared_ptr<ServerPlayer> player)
{
    return GameType::LOBBY;
}

bool MasterGameMode::IsInRound()
{
    return mInternalState >= eInternalGameModeState_RoundStarting &&
           mInternalState <= eInternalGameModeState_RoundEnding;
}

void MasterGameMode::PlaceSpectators(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnSpawn(boost::shared_ptr<ServerPlayer> player, boost::shared_ptr<ServerPlayer> owner, bool joining)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnSpawned(boost::shared_ptr<ServerPlayer> player, bool joining)
{
    mPlaylistDirty = true;
}

void MasterGameMode::SendPlaylistInfo(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::SetLockPlayerPosition(boost::shared_ptr<ServerPlayer> player, bool locked)
{
    mLockPlayerPositions = locked && player;
}

void MasterGameMode::OnJoin(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnFollow(boost::shared_ptr<ServerPlayer> player, bool started, bool force)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnLeave(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnDeath(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::StopAchievementExploit(const std::wstring& playerName)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnKickReqested(int playerIndex, int reason)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnStartSpectating(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnServerResume(long long suspendedTime)
{
    mPlaylistDirty = true;
}

void MasterGameMode::PlayerFinishedLoadingLocally()
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetLobbyTimerAfterSuspend()
{
    System::processTimeInMilliSecs();
}

void MasterGameMode::OnLevelLoad(ServerLevel* level)
{
    mPlaylistDirty = true;
}

int MasterGameMode::GetChunkHistory() const
{
    return mChunkHistory;
}

int MasterGameMode::GetRoundTime()
{
    return mRoundTimeOffset +
           static_cast<int>(GetCurrentTimeMillis() - mRoundStartTime);
}

int MasterGameMode::GetPostGameTime()
{
    return mPostGameTime;
}

int MasterGameMode::GetMaxKills()
{
    return mInternalState;
}

int MasterGameMode::GetActiveTeamCount()
{
    return mActiveTeamCount;
}

Vec3* MasterGameMode::GetLastKnownPosition(boost::shared_ptr<Player> player)
{
    return mLastKnownPositions[0];
}

AABB* MasterGameMode::GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player)
{
    Vec3* position = GetLastKnownPosition(player);
    if (position == 0)
        return 0;
    return AABB::newTemp(position->x, position->y, position->z,
                         position->x, position->y, position->z);
}

unsigned int MasterGameMode::GetLastCheckpointID(const StatsUID& uid)
{
    return mGameStats.GetLastCheckpointID(uid);
}

void MasterGameMode::CatchUpCheckpointCheck(boost::shared_ptr<ServerPlayer> player, AABB* sweptVolume)
{
    mPlaylistDirty = true;
}

void MasterGameMode::KillTimers()
{
    for (TimerMap::iterator entry = mTimers.begin(); entry != mTimers.end(); ++entry)
        entry->second.active = false;
}

void MasterGameMode::PositionPlayersInSpectatorLobby()
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetPlayer(boost::shared_ptr<ServerPlayer> player)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ReduceShowdownTime(int amount, int minimum)
{
    bool timerAdjusted = false;
    int remaining = 0;

    if (HasTimer(eTimerID_RoundLimit))
    {
        remaining = GetTimerRemainingTime(eTimerID_RoundLimit);
        if (minimum < remaining)
        {
            if (remaining - amount < minimum)
            {
                amount = remaining - minimum;
                RestartTimer(eTimerID_RoundLimit, minimum);
                timerAdjusted = true;
            }
            else
            {
                RestartTimer(eTimerID_RoundLimit, remaining - amount);
                timerAdjusted = true;
            }
        }
    }

    if (HasTimer(eTimerID_Tutorial))
    {
        remaining = GetTimerRemainingTime(eTimerID_Tutorial);
        if (minimum < remaining)
        {
            if (remaining - amount < minimum)
            {
                amount = remaining - minimum;
                RestartTimer(eTimerID_Tutorial, minimum);
                timerAdjusted = true;
            }
            else
            {
                RestartTimer(eTimerID_Tutorial, remaining - amount);
                timerAdjusted = true;
            }
        }
    }

    if ((mRoundTimeOffset != 0 || mPausedRoundTime != 0) && timerAdjusted)
    {
        const unsigned int low = static_cast<unsigned int>(mPausedRoundTime);
        mPausedRoundTime = static_cast<int>(low - amount);
        mRoundTimeOffset -= static_cast<int>((amount >> 31) + static_cast<unsigned int>(low < static_cast<unsigned int>(amount)));
    }
}

void MasterGameMode::SetDefaults(LaunchMoreOptionsMenuInitData& data)
{
    System::processTimeInMilliSecs();
}

void MasterGameMode::SaveDefaults(const LaunchMoreOptionsMenuInitData& data)
{
    System::processTimeInMilliSecs();
}

const std::vector<int>& MasterGameMode::GetOptionsList(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionsList(option);
}

unsigned int MasterGameMode::GetOptionMin(EMiniGameId miniGame, eCustomGameModeHostOption option, bool includeRandom)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionMin(option, includeRandom);
}

int MasterGameMode::GetOptionMax(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionMax(option);
}

std::wstring MasterGameMode::GetValueText(EMiniGameId miniGame, eCustomGameModeHostOption option, int value)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetValueText(option, value);
}

void MasterGameMode::UpdateRulePlayList(std::vector<LevelRules::LevelRulesetGroup*>* playlist)
{
    mPlaylist.insert(mPlaylist.end(), playlist->begin(), playlist->end());
    std::sort(mPlaylist.begin(), mPlaylist.end(), LevelRules::LevelRulesetGroup::SortMapRules);
    mRuleGroups.clear();
    mShuffledRuleGroups.clear();
    mShuffledRuleGroups.insert(mShuffledRuleGroups.end(), mPlaylist.begin(), mPlaylist.end());
}

void MasterGameMode::SetPlayerReady(unsigned int playerIndex, bool ready)
{
    if (playerIndex < 8)
    {
        if (mPlayerReady[playerIndex] != ready)
            mReadyPlayers += ready ? 1 : -1;
        mPlayerReady[playerIndex] = ready;
    }
}

bool MasterGameMode::IsChunkWithinLobbyRegionOfRoundLevel(int chunkX, int chunkZ)
{
    return mRoundStarted || mRoundEnding;
}

void MasterGameMode::GetMaxActiveBounds(int& minimumX, int& minimumZ, int& maximumX, int& maximumZ)
{
    if (!mTeamAreas.empty())
    {
        std::vector<AABB*>::const_iterator it = mTeamAreas.begin();
        const AABB* bounds = *it;
        minimumX = static_cast<int>(bounds->mMinX);
        minimumZ = static_cast<int>(bounds->mMinZ);
        maximumX = static_cast<int>(bounds->mMaxX);
        maximumZ = static_cast<int>(bounds->mMaxZ);
        for (++it; it != mTeamAreas.end(); ++it)
        {
            bounds = *it;
            if (static_cast<int>(bounds->mMinX) < minimumX)
                minimumX = static_cast<int>(bounds->mMinX);
            if (static_cast<int>(bounds->mMinZ) < minimumZ)
                minimumZ = static_cast<int>(bounds->mMinZ);
            if (maximumX < static_cast<int>(bounds->mMaxX))
                maximumX = static_cast<int>(bounds->mMaxX);
            if (maximumZ < static_cast<int>(bounds->mMaxZ))
                maximumZ = static_cast<int>(bounds->mMaxZ);
        }
    }
}

void MasterGameMode::BlockDestroyedAt(const BlockPos& position)
{
    mPlaylistDirty = true;
}

void MasterGameMode::ResetSpawnLayers()
{
    mSpawnLayerIndex = 0;
}

void MasterGameMode::RegenTumbleLayers()
{
    ResetSpawnLayers();
    RestartMapGenerator();
}

void MasterGameMode::RestartMapGenerator()
{
    mPlaylistDirty = true;
    CommonMasterGameMode::RestartMapGenerator();
}

unsigned int MasterGameMode::GetGameModeState() const
{
    return static_cast<unsigned int>(mInternalState);
}

ServerLevel* MasterGameMode::GeneratorTargetLevel() const
{
    return mRoundLevel;
}

void MasterGameMode::setChosenThemeWordId(int wordId)
{
    mChosenThemeWordId = wordId;
    CommonMasterGameMode::setChosenThemeWordId(wordId);
}

bool MasterGameMode::IsRoundPlaying() const
{
    return mInternalState == eInternalGameModeState_RoundPlaying;
}

Scoreboard* MasterGameMode::getScoreboard()
{
    return mScoreboard;
}

void MasterGameMode::OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpoint, CheckpointRuleDefinition* rule)
{
    mPlaylistDirty = true;
}

void MasterGameMode::PlayReachedEndSFX(const std::vector<boost::shared_ptr<ServerPlayer> >& players, boost::shared_ptr<ServerPlayer> winner)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* rule)
{
    mPlaylistDirty = true;
}

void MasterGameMode::OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int powerup, PowerupRuleDefinition* rule)
{
    mPlaylistDirty = true;
}

MasterGameMode::ChestRefillInfo::ChestRefillInfo(const ChestRefillInfo& other)
    : distance(other.distance), weight(other.weight), x(other.x), y(other.y), z(other.z), itemSet(other.itemSet), minimumPlayers(other.minimumPlayers), maximumPlayers(other.maximumPlayers), refillCount(other.refillCount), refillInterval(other.refillInterval), nextRefill(other.nextRefill), region(other.region), chestIndex(other.chestIndex), lootGroup(other.lootGroup)
{}

MasterGameMode::CountdownInfo::CountdownInfo()
    : message(0), arguments(), callbackData(0), completeCallback(0), tickCallback(0)
{}
