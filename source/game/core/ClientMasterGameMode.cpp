#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientVoteables.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Team.hpp"
#include "game/core/GhostController.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/Voteable.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/world/generation/Player.hpp"
#include <algorithm>

void Sleep(unsigned int milliseconds);

unsigned int WaitForNextState(void* context)
{
    ClientMasterGameMode* gameMode = static_cast<ClientMasterGameMode*>(context);
    while (gameMode->IsRoundRestarting())
        Sleep(5);
    return 0;
}

ClientMasterGameMode::MapSelectInfo::MapSelectInfo()
    : uniqueId(0), name(), mapSize(0)
{
}

ClientMasterGameMode::MapSelectInfo::MapSelectInfo(const MapSelectInfo& other)
    : uniqueId(other.uniqueId), name(other.name), mapSize(other.mapSize)
{
}

ClientMasterGameMode::ClientMasterGameMode()
    : CommonMasterGameMode(),
      mNewLevelDataPending(false),
      mPlaylist(),
      mStringTable(),
      mGameModeState(0),
      mTransitions(),
      mStrings(),
      mPlayerProgress(),
      mLastCheckpoints(),
      mLocalRoundTimeStart(0),
      mServerIsWaitingOnChunkLoading(false),
      mLocalChunksLoaded(false),
      mBRUPsRemaining(0),
      mBRUPCount(0),
      mLoadedChunkCount(0),
      mChunkLoadProgress(0),
      mLastChunkLoadProgress(0x7fffffff)
{
    mVoteables = new ClientVoteables();
    mVoteables->addVoteable(new Voteable(0, false, true));
    mVoteables->addVoteable(new Voteable(1, false, false));
    mVoteables->addVoteable(new Voteable(2, false, false));
    mVoteables->addVoteable(new Voteable(3, false, true));
    ResetReadyStates();
}

ClientMasterGameMode::~ClientMasterGameMode()
{
    delete mVoteables;
}

bool ClientMasterGameMode::IsRoundRestarting() const { return GetGameModeState() == 10; }

void ClientMasterGameMode::ResetReadyStates()
{
    for (int i = 0; i < 8; ++i) mPlayerReady[i] = false;
}

void ClientMasterGameMode::Reset()
{
    mRoundInfo.reset();
    mNewLevelDataPending = false;
    mCurrentLevelGenOptions = NULL;
}

void ClientMasterGameMode::PlaySound(const SoundEvent* sound, float volume, float pitch)
{
    Minecraft::GetInstance()->mSoundEngine->playUI(sound, volume, pitch);
}

void ClientMasterGameMode::OnRoundPlay() { mGameModeState = 7; }
void ClientMasterGameMode::OnRoundEnding() { mGameModeState = 9; }
void ClientMasterGameMode::OnRoundWaitingToStart() { mGameModeState = 5; mPlayerProgress.clear(); }
void ClientMasterGameMode::OnRoundStart() { mGameModeState = 6; }

void ClientMasterGameMode::OnRoundEndingToLobby()
{
    mRoundInfo.reset();
    mLocalRoundTimeStart = 0;
}

void ClientMasterGameMode::OnStateChange(EGameModeState state) { mGameModeState = static_cast<unsigned int>(state); }

void ClientMasterGameMode::OnExitedGame()
{
    CommonMasterGameMode::OnExitedGame();
    OnStateChange(EGameModeStateDefault);
}

void ClientMasterGameMode::ClearPlaylist() { mPlaylist.clear(); }

void ClientMasterGameMode::AddMapToPlaylist(int uniqueId, const std::wstring& name, int mapSize)
{
    MapSelectInfo info;
    info.uniqueId = uniqueId;
    info.name = name;
    info.mapSize = mapSize;
    mPlaylist.push_back(info);
}

void ClientMasterGameMode::SetMapVote(int playerIndex, int vote, bool remove)
{
    Voteable* voteable = mVoteables->getVoteable(0);
    if (voteable != NULL) voteable->setVoteAndSync(playerIndex, vote, remove ? -1 : 1);
}

unsigned int ClientMasterGameMode::GetMapVote(int playerIndex, int vote)
{
    Voteable* voteable = mVoteables->getVoteable(0);
    return voteable == NULL ? 0U : static_cast<unsigned int>(voteable->getVote(playerIndex, vote));
}

void ClientMasterGameMode::GetUniquePlaylist(std::vector<MapSelectInfo>& playlist)
{
    playlist.clear();
    std::vector<int> themes;
    for (std::vector<MapSelectInfo>::const_iterator it = mPlaylist.begin(); it != mPlaylist.end(); ++it)
    {
        if (std::find(themes.begin(), themes.end(), it->uniqueId) != themes.end()) continue;
        themes.push_back(it->uniqueId);
        playlist.push_back(*it);
    }
}

void ClientMasterGameMode::GetCurrentMapVotesText(std::vector<HtmlString>& text) { text.clear(); }
void ClientMasterGameMode::GetCurrentVariantVotesText(std::vector<HtmlString>& text)
{
    text.clear();
    Voteable* voteable = mVoteables->getVoteable(0);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    MiniGameDef* definition =
        MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(GetNextGameMode()), true);
    int count = static_cast<int>(definition->GetGameVariantCount()) - 2;
    if (definition->IsCustomGame())
        count = definition->GetLootSetCount();
    for (int i = 0; i < count; ++i)
    {
        const std::wstring label = definition->GetLootSetText(i);
        boost::unordered_map<int, int>::const_iterator it = totals.find(i);
        if (it == totals.end())
            text.push_back(HtmlString(formatwstr(L"%ls : 0", label.c_str()),
                                      static_cast<eMinecraftColour>(0), false, false, 0));
        else
            text.push_back(HtmlString(formatwstr(L"%ls : %d", label.c_str(), it->second),
                                      static_cast<eMinecraftColour>(0), false, false, 0));
    }
}

void ClientMasterGameMode::SetKickVote(int playerIndex, unsigned char colour, bool vote)
{
    Voteable* voteable = mVoteables->getVoteable(1);
    if (voteable == NULL) return;
    voteable->setVoteAndSync(playerIndex, colour, vote ? 1 : -1);
}

unsigned int ClientMasterGameMode::GetKickVote(int playerIndex, unsigned char colour)
{
    Voteable* voteable = mVoteables->getVoteable(1);
    return voteable == NULL ? 0U : static_cast<unsigned int>(voteable->getVote(playerIndex, colour));
}

void ClientMasterGameMode::SetBuildOffScoringVote(int playerIndex, int vote, int value)
{
    Voteable* voteable = mVoteables->getVoteable(2);
    if (voteable == NULL) return;
    voteable->setVoteAndSync(playerIndex, vote, static_cast<char>(value));
}

unsigned int ClientMasterGameMode::GetBuildOffTeamIndexToScore(int playerIndex)
{
    boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(playerIndex);
    if (player != NULL)
    {
        Vec3* position = player->getPos(1.0f);
        Team* team = GetTeamAtPosition(position->x, position->y, position->z, 6.0);
        if (team != NULL && team != player->getTeam())
            return team->getTeamIndex();
    }

    return 0xffffffffU;
}

unsigned int ClientMasterGameMode::GetBuildOffScoringVote(int playerIndex, int vote)
{
    Voteable* voteable = mVoteables->getVoteable(2);
    return voteable == NULL ? 0U : static_cast<unsigned int>(voteable->getVote(playerIndex, vote));
}

void ClientMasterGameMode::SetBuildOffThemeVote(int playerIndex, int vote, int value)
{
    Voteable* voteable = mVoteables->getVoteable(3);
    if (voteable != NULL) voteable->setVoteAndSync(playerIndex, vote, static_cast<char>(value));
}

unsigned int ClientMasterGameMode::GetBuildOffThemeVote(int playerIndex)
{
    Voteable* voteable = mVoteables->getVoteable(3);
    return voteable == NULL ? 0U : static_cast<unsigned int>(voteable->getVote(playerIndex, 0));
}

void ClientMasterGameMode::GetString(const int& id) { mStringTable.getString(id); }
void ClientMasterGameMode::GetCurrentThemeVotesText(std::vector<HtmlString>& text) { text.clear(); }

std::wstring ClientMasterGameMode::GetTransitionTextOverride()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame == NULL) return std::wstring();
    return miniGame->GetLootSetText();
}

void ClientMasterGameMode::SetPlayerReady(unsigned int playerIndex, bool ready)
{
    if (playerIndex < 8) mPlayerReady[playerIndex] = ready;
}

bool ClientMasterGameMode::IsPlayerReady(unsigned int playerIndex)
{
    return playerIndex < 8 && mPlayerReady[playerIndex];
}

void ClientMasterGameMode::SetCurrentTheme(int themeId, int subId)
{
    mRoundInfo.setTheme(themeId, subId);
}

void ClientMasterGameMode::SetCurrentUniqueId(int uniqueId)
{
    if (uniqueId >= 0)
        mRoundInfo.setUniqueThemeId(uniqueId);
    else
        mRoundInfo.reset();
}

unsigned int ClientMasterGameMode::GetCurrentThemeId() const { return mRoundInfo.getThemeId(); }
unsigned int ClientMasterGameMode::GetCurrentUniqueId() const { return mRoundInfo.getThemeUniqueId(); }

void ClientMasterGameMode::SetNextLevelGenerationOptions(LevelGenerationOptions* options, bool owned)
{
    mNewLevelDataPending = owned;
    mCurrentLevelGenOptions = options;
    if (owned)
        mCurrentLevelGenOptions->loadBaseSaveData();
    mTeamAreas.clear();
    mCurrentLevelGenOptions->getRequiredGameRules()->getActiveChunkAreas(&mTeamAreas);
}

void ClientMasterGameMode::OnShowdown()
{
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == NULL)
        return;

    const int id = miniGame->GetId();
    const SoundEvent* event = NULL;
    if (id == 1)
        event = SoundEvents::MG01_SHOWDOWN;
    else if (id == 2 || id == 4)
        event = SoundEvents::MG02_SHOWDOWN;
    else
        return;

    const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnShowdown);
    PlaySound(event, static_cast<float>(sound.Volume()), 1.0f);
}
void ClientMasterGameMode::OnChestRefill()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame == NULL)
        return;
    const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnChestRefill);
    PlaySound(SoundEvents::MG01_CHEST, static_cast<float>(sound.Volume()), 1.0f);
}
void ClientMasterGameMode::OnDeath(const Vec3* position)
{
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == NULL)
        return;

    const float pitchStep =
        static_cast<float>(8 - Minecraft::GetInstance()->mLevel->GetClientScoreboard()->GetActivePlayerCount(
                                   static_cast<EMiniGameId>(15))) *
        0.125f * 0.25f;
    const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);

    switch (miniGame->GetId())
    {
    case 1:
        PlaySound(SoundEvents::MG01_DEATH, static_cast<float>(sound.Volume()) - pitchStep,
                  1.0f - pitchStep);
        break;
    case 2:
        miniGame->SetDisabledEditActions(false);
        PlaySound(SoundEvents::MG02_DEATH, static_cast<float>(sound.Volume()), 1.0f);
        break;
    case 3:
        if (position != NULL)
        {
            Minecraft::GetInstance()->mSoundEngine->play(
                SoundEvents::MG03_DEATH, static_cast<float>(position->x),
                static_cast<float>(position->y), static_cast<float>(position->z),
                static_cast<float>(sound.Volume()), 1.0f, NULL);
        }
        break;
    }
}

void ClientMasterGameMode::DoLobbyTickSound(int tick, const MiniGameDef::SoundSpec& sound)
{
    if (tick > static_cast<int>(sound.ForLast()))
        return;

    const SoundEvent* event = SoundEvents::MG01_LOBBY;
    if (tick == 1)
        event = SoundEvents::MG01_LOBBY_ZERO;
    else if ((tick & 1) == 0)
        event = SoundEvents::MG01_LOBBY_ACCENT;

    PlaySound(event, static_cast<float>(sound.Volume()), 1.0f);
}

void ClientMasterGameMode::DoRoundStartTickSound(int tick, const MiniGameDef::SoundSpec& sound)
{
    if (tick <= static_cast<int>(sound.ForLast()))
    {
        const SoundEvent* event = GetMiniGame()->GetId() == 2
                                      ? SoundEvents::MG02_START_COUNTDOWN
                                      : SoundEvents::MG01_START_COUNTDOWN;
        PlaySound(event, static_cast<float>(sound.Volume()), 1.0f);
    }
}

void ClientMasterGameMode::OnRoundStartTimerTick(int tick)
{
    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true) || tick != 3)
        return;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        if (gConsoleUIController.IsSceneInStack(playerIndex, static_cast<EUIScene>(60)))
            gConsoleUIController.CloseUIScenes(playerIndex, true);
    }
}

void ClientMasterGameMode::DoGraceTickSound(int tick, const MiniGameDef::SoundSpec& sound)
{
    if (tick <= static_cast<int>(sound.ForLast()))
    {
        PlaySound(tick == 0 ? SoundEvents::MG01_GRACE_ZERO : SoundEvents::MG01_GRACE,
                  static_cast<float>(sound.Volume()), 1.0f);
    }
}

void ClientMasterGameMode::OnChatPacket(boost::shared_ptr<ClientboundChatPacket>) { }
void ClientMasterGameMode::SetGeneratorSeed(long long seed, float scale) { UpdateGeneratorSeed(seed, scale); }
bool ClientMasterGameMode::IsWaitingToStart() const { return GetGameModeState() == 5; }
bool ClientMasterGameMode::IsRoundStarting() const { return GetGameModeState() == 6; }
bool ClientMasterGameMode::IsWaitingForMusicStartEvent() const
{
    if (!IsWaitingToStart() && !IsRoundStarting() && mGameModeState != 3 && mGameModeState != 4)
        return false;
    return true;
}
bool ClientMasterGameMode::IsRoundEnding() const { return GetGameModeState() == 9; }
bool ClientMasterGameMode::IsRoundPlaying() const { return GetGameModeState() == 7; }
bool ClientMasterGameMode::IsInScoringState() const { return Minecraft::InMiniGame(static_cast<EMiniGameId>(4), true) && GetGameModeState() == 8; }
bool ClientMasterGameMode::IsInLobbyState() { return GetGameModeState() <= 4; }
bool ClientMasterGameMode::IsInActiveRoundState() { return GetGameModeState() >= 6 && GetGameModeState() <= 10; }
bool ClientMasterGameMode::IsInTrasitionFromLobby()
{
    const unsigned int state = mGameModeState;
    if ((state != 0) && ((state < 3) || (state > 4)))
        return false;
    return true;
}
bool ClientMasterGameMode::CheckLocalChunksLoaded() { return mLocalChunksLoaded; }
void ClientMasterGameMode::FireGameRoundStats()
{
    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true))
        return;
    LevelGenerationOptions* options = g_pApp != NULL ? g_pApp->getLevelGenerationOptions() : NULL;
    if (options == NULL)
        options = mCurrentLevelGenOptions;
    const MiniGameDef* current = GetMiniGame();
    if (current == NULL)
        return;
    MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
        static_cast<EMiniGameId>(current->GetId()), true);
    if (definition == NULL || !definition->IsCRCValid() || options == NULL || !options->isCRCValid())
        return;

    ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
    GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
    if (scoreboard == NULL || ghosts == NULL)
        return;
    for (int slot = 0; slot < 4; ++slot)
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(slot);
        if (!player)
            continue;
        StatsUID uid = player->getSUID();
        const int uniqueId = static_cast<int>(options->getUniqueId());
        if (!ghosts->mRecordingActive[slot] || player->isSpectator())
            continue;
        unsigned int lifespan = static_cast<unsigned int>(scoreboard->GetLifeSpan(uid, -1));
        int recorded = 0;
        if (definition->ArePointsActive())
        {
            int points = scoreboard->GetPoints(uid, -1);
            recorded = ghosts->RecordScore(slot, uniqueId, static_cast<unsigned short>(points), lifespan);
        }
        else
        {
            recorded = ghosts->RecordTime(slot, uniqueId, lifespan);
        }
        if (recorded != 0)
            scoreboard->SetPersonalBest(uid, -1);
    }
}
void ClientMasterGameMode::updateProgressRenderer() { }
void ClientMasterGameMode::Tick() { CheckLocalChunksLoaded(); updateProgressRenderer(); }
bool ClientMasterGameMode::isNewLevelDataPending() { return mNewLevelDataPending; }
bool ClientMasterGameMode::isWaitingToTransition() { return !mTransitions.empty() && IsInActiveRoundState(); }

GameModeTransitionParams* ClientMasterGameMode::popTransition()
{
    if (mTransitions.empty()) return NULL;
    GameModeTransitionParams* transition = mTransitions.front();
    mTransitions.erase(mTransitions.begin());
    return transition;
}

void ClientMasterGameMode::queueTransition(GameModeTransitionParams* transition)
{
    if (transition != NULL) mTransitions.push_back(transition);
}

void ClientMasterGameMode::dropAllTransitions()
{
    for (std::vector<GameModeTransitionParams*>::iterator it = mTransitions.begin(); it != mTransitions.end(); ++it) delete *it;
    mTransitions.clear();
}

void ClientMasterGameMode::onChangeCustomGameMode()
{
    if (g_pApp == NULL)
        return;

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options == NULL)
        return;

    LevelRuleset* rules = options->getRequiredGameRules();
    if (rules == NULL)
        return;

    std::vector<CheckpointRuleDefinition*>* checkpoints = rules->getCheckpoints();
    if (checkpoints != NULL)
        setCheckpoints(*checkpoints);

    std::vector<TargetAreaRuleDefinition*> targetAreas;
    rules->getTargetAreas(&targetAreas);
    mTeamAreas.clear();
    for (std::vector<TargetAreaRuleDefinition*>::const_iterator it = targetAreas.begin();
         it != targetAreas.end(); ++it)
    {
        if (*it != NULL && (*it)->mPrimary)
            mTeamAreas.push_back((*it)->getArea());
    }
}
void ClientMasterGameMode::SetServerIsWaitingOnChunkLoading() { mServerIsWaitingOnChunkLoading = true; }
bool ClientMasterGameMode::ServerIsWaitingOnUsLoading() { return mServerIsWaitingOnChunkLoading; }
void ClientMasterGameMode::RecievedBRUP() { if (mBRUPsRemaining > 0) --mBRUPsRemaining; }
void ClientMasterGameMode::SetBRUPSToWaitOn(int count) { mBRUPCount = count; mBRUPsRemaining = count; }
bool ClientMasterGameMode::IsValidStateForItemUse() const { return !IsWaitingToStart() && !IsRoundStarting() && !IsRoundRestarting() && GetGameModeState() != 4; }
unsigned int ClientMasterGameMode::GetGameModeState() const { return mGameModeState; }
MultiPlayerLevel* ClientMasterGameMode::GeneratorTargetLevel() const { return Minecraft::GetInstance()->mLevels[0]; }
Scoreboard* ClientMasterGameMode::getScoreboard() { return Minecraft::GetInstance()->mLevel->GetClientScoreboard(); }

double ClientMasterGameMode::GetPlayerMapProgress(PlayerUID uid) const
{
    boost::unordered_map<PlayerUID, double, boost::hash<PlayerUID> >::const_iterator it = mPlayerProgress.find(uid);
    return it == mPlayerProgress.end() ? 0.0 : it->second;
}

void ClientMasterGameMode::OnProgressMade(const boost::shared_ptr<Player>& player, double progress)
{
    if (player->GetType() != 0x000000000C800004ULL)
        mPlayerProgress[player->getSUID().GetID()] = progress;
}

void ClientMasterGameMode::ResetMapDistances() { CommonMasterGameMode::ResetMapDistances(); mPlayerProgress.clear(); mLastCheckpoints.clear(); }
int ClientMasterGameMode::convertCheckpointIDToSpawnableID(int checkpoint) { return CheckpointIDtoSequenceID(checkpoint); }
void ClientMasterGameMode::SetLocalRoundTimeStart(long long value) { mLocalRoundTimeStart = value; }
void ClientMasterGameMode::SetLocalRoundTimeStartOverride(long long value) { mLocalRoundTimeStart = value; }
unsigned long long ClientMasterGameMode::GetLocalRoundTimeStart() const { return static_cast<unsigned long long>(mLocalRoundTimeStart); }
AABB* ClientMasterGameMode::GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player) { return player->getSweptVolume(); }
unsigned int ClientMasterGameMode::GetLastCheckpointID(const StatsUID& uid)
{
    boost::unordered_map<StatsUID, int, boost::hash<StatsUID> >::const_iterator it =
        mLastCheckpoints.find(uid);
    return it == mLastCheckpoints.end() ? 0xffffffffU : static_cast<unsigned int>(it->second);
}

unsigned int ClientMasterGameMode::SetLastCheckpointID(const StatsUID& uid, int checkpoint, bool notify)
{
    const int sequence = CheckpointIDtoSequenceID(checkpoint);
    const int lastSequence = CheckpointIDtoSequenceID(GetLastCheckpointID(uid));
    if (!notify && sequence != lastSequence + 1)
        return 0;

    mLastCheckpoints[uid] = checkpoint;
    return 1;
}
