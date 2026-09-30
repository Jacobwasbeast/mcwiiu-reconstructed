#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/ClientVoteables.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/core/Team.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Voteable.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include <algorithm>

namespace
{
}

int WaitForNextState(void* context)
{
    ClientMasterGameMode* gameMode = static_cast<ClientMasterGameMode*>(context);
    while (gameMode->IsRoundRestarting())
        Sleep(5);
    return 0;
}

ClientMasterGameMode::MapSelectInfo::MapSelectInfo()
    : uniqueId(0), name(), texturePackId(0)
{
}

ClientMasterGameMode::MapSelectInfo::MapSelectInfo(const MapSelectInfo& other)
    : uniqueId(other.uniqueId), name(other.name), texturePackId(other.texturePackId)
{
}

ClientMasterGameMode::ClientMasterGameMode()
    : CommonMasterGameMode(),
      mNewLevelDataPending(false),
      mPlaylist(),
      mStringTable(),
      mGameModeState(eGameModeState_None),
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
      mSubRoundLoadStartTime(0x7fffffff)
{
    mVoteables = new ClientVoteables();
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_Map, false, true));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_Kick, false, false));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_BuildOffScoring, false, false));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_BuildOffTheme, false, true));
    ResetReadyStates();
}

ClientMasterGameMode::~ClientMasterGameMode()
{
    delete mVoteables;
}

bool ClientMasterGameMode::IsRoundRestarting() const { return GetGameModeState() == eGameModeState_LoadingSubRound; }

void ClientMasterGameMode::ResetReadyStates()
{
    for (int i = 0; i < 8; ++i) mPlayerReady[i] = false;
}

void ClientMasterGameMode::Reset()
{
    mRoundInfo.reset();
    mNewLevelDataPending = false;
    mNextLevelGenOptions = NULL;
}

void ClientMasterGameMode::PlaySound(const SoundEvent* sound, float volume, float pitch)
{
    Minecraft::GetInstance()->mSoundEngine->playUI(sound, volume, pitch);
}

void ClientMasterGameMode::OnRoundWaitingToStart()
{
    mLocalChunksLoaded = false;
    mPlayerProgress.clear();
    mLocalRoundTimeStart = 0;
    mSubRoundLoadStartTime = 0x7fffffff;

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
            if (localPlayer != NULL)
            {
                localPlayer->SetWaitingForScoreboard(true);
                localPlayer->ResetThermalVariables();
            }
        }
    }
}

void ClientMasterGameMode::OnRoundStart()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
        boost::shared_ptr<Player> player = localPlayer;
        if (!player)
        {
            mPlayerReady[playerIndex] = false;
            continue;
        }

        player->mInventory->m_selected = 0;
        mPlayerReady[playerIndex] = true;
    }

    MiniGameDef* miniGame = const_cast<MiniGameDef*>(GetMiniGame());
    if (miniGame != NULL && miniGame->GetId() == static_cast<int>(eMiniGameId_Tumble))
    {
        miniGame->SetDisabledEditActions(true);
        static_cast<ConsoleSoundEngine*>(minecraft->mSoundEngine)->LimitMusicVolume();
    }

    ResetMapDistances();

    if (miniGame != NULL && miniGame->HasFeatureStartCameraAnimation())
    {
        Random random;
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
            boost::shared_ptr<Player> player = localPlayer;
            if (!player)
                continue;

            CameraController* controller = new CameraController(3);
            if (random.nextBoolean())
            {
                controller->addNode(0.0, 0.0, -25.0, 35.0, 180.0, 0.0, 0.0f);
                controller->addNode(0.0, 0.0, -15.0, 35.0, 0.0, 0.0, 150.0f);
            }
            else
            {
                controller->addNode(0.0, 0.0, -25.0, 35.0, -180.0, 0.0, 0.0f);
                controller->addNode(0.0, 0.0, -15.0, 35.0, 0.0, 0.0, 150.0f);
            }
            controller->attachToPlayer(player, true);
        }
    }
}

void ClientMasterGameMode::OnRoundPlay()
{
    mLocalRoundTimeStart = System::processTimeInMilliSecs();

    Minecraft* minecraft = Minecraft::GetInstance();
    MiniGameDef* miniGame = const_cast<MiniGameDef*>(GetMiniGame());
    minecraft->mSoundEngine->FlagMusicEventStart();

    if (miniGame == NULL)
        return;

    const int miniGameId = miniGame->GetId();
    if (miniGameId != 0)
    {
        if (miniGameId != 1)
        {
            if (miniGameId < 3)
            {
                miniGame->SetDisabledEditActions(false);
                static_cast<ConsoleSoundEngine*>(minecraft->mSoundEngine)->RemoveMusicVolumeLimit();
                const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
                PlaySound(SoundEvents::MG02_GAME_START, static_cast<float>(sound.Volume()), 1.0f);
                return;
            }
            if (miniGameId == 3)
            {
                const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
                PlaySound(SoundEvents::MG03_COUNTDOWN_ZERO, static_cast<float>(sound.Volume()), 1.0f);

                GhostController* ghosts = minecraft->GetGhostController();
                if (ghosts != NULL)
                {
                    int ghostSlot = 0;
                    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                            minecraft->mLocalPlayers[playerIndex];
                        boost::shared_ptr<Player> player = localPlayer;
                        if (player != NULL && !player->isSpectator())
                        {
                            Vec3* pos = player->getPos(1.0f);
                            ghosts->startRecording(ghostSlot, pos);
                        }
                        ++ghostSlot;
                    }
                    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
                        ghosts->startPlayback();
                }
                return;
            }
            if (miniGameId != 4)
                return;
        }

        const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG01_GAME_START, static_cast<float>(sound.Volume()), 1.0f);
    }
}

void ClientMasterGameMode::OnRoundEnding()
{
    MiniGameDef* miniGame = const_cast<MiniGameDef*>(GetMiniGame());
    if (miniGame != NULL)
    {
        const int miniGameId = miniGame->GetId();
        if (miniGameId == 1 || miniGameId == 4)
        {
            const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnRoundEnding);
            PlaySound(SoundEvents::MG01_CELEBRATION, static_cast<float>(sound.Volume()), 1.0f);
        }
        else if (miniGameId == 2)
        {
            miniGame->SetDisabledEditActions(false);
            const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnRoundEnding);
            PlaySound(SoundEvents::MG02_GAME_END, static_cast<float>(sound.Volume()), 1.0f);
        }
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    ClientScoreboard* scoreboard = minecraft->mLevel->GetClientScoreboard();
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
        boost::shared_ptr<Player> player = localPlayer;
        if (!player)
            continue;

        const StatsUID suid = player->getSUID();
        if (!scoreboard->IsWinner(suid, -1))
        {
            gConsoleUIController.CloseUIScenes(playerIndex, true);
            if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
                ++mStrings[player->getDisplayName()];
        }
        else if (Minecraft::InMiniGame(eMiniGameId_Battle, true) &&
                 CGameNetworkManager::sInstance.IsInPublicJoinableGame())
        {
            player->awardStat(GenericStats::theStudent(), GenericStats::param_theStudent());
            const unsigned int winCount = ++mStrings[player->getDisplayName()];
            if (winCount > 2)
                player->awardStat(GenericStats::hasBecomeTheMaster(),
                                  GenericStats::param_hasBecomeTheMaster());
        }
    }

    if (telemetryManagerInstance != NULL)
        telemetryManagerInstance->RecordGameModeRoundFinished();
}

void ClientMasterGameMode::OnRoundEndingToLobby()
{
    mRoundInfo.reset();
    mLocalRoundTimeStart = 0;
}

void ClientMasterGameMode::OnStateChange(EGameModeState state)
{
    const EGameModeState previousState = mGameModeState;
    if (previousState == state)
        return;

    mGameModeState = state;
    if (previousState == eGameModeState_None)
    {
        ResetMapDistances();
        if (!CGameNetworkManager::sInstance.IsHost())
            CMinecraftApp::SetupMiniGameMusic();
        return;
    }

    switch (state)
    {
    case eGameModeState_None:
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        minecraft->SetGhostController(NULL);
        MiniGameDef* definition =
            MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true);
        if (definition != NULL)
            minecraft->SetupMiniGameInstance(*definition, 0);
        mStrings.clear();
        Reset();
        return;
    }
    case eGameModeState_Lobby:
        if (previousState == eGameModeState_RoundEnding)
            OnRoundEndingToLobby();
        return;
    case eGameModeState_SelectingGameRules:
    {
        MultiPlayerLevel* level = static_cast<MultiPlayerLevel*>(Minecraft::GetInstance()->mLevel);
        ClientScoreboard* scoreboard = level->GetClientScoreboard();
        scoreboard->mCurrentRound = 0;
        Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
        if (voteable != NULL)
            voteable->clearVotes();
        return;
    }
    case eGameModeState_WaitingToStart:
        OnRoundWaitingToStart();
        return;
    case eGameModeState_RoundStarting:
        OnRoundStart();
        return;
    case eGameModeState_RoundPlaying:
        OnRoundPlay();
        return;
    case eGameModeState_RoundEnding:
        OnRoundEnding();
        return;
    case eGameModeState_LoadingSubRound:
    {
        CleanUpMapGenerator();
        gConsoleUIController.CloseAllPlayersScenes(true);
        gConsoleUIController.HideAllGameUIElements();

        _LoadingInputParams* params = new _LoadingInputParams();
        params->m_threadProc = WaitForNextState;
        params->m_threadParam = this;
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame != NULL)
            params->m_miniGameId = miniGame->GetId();

        FullscreenProgressState* progressState = new FullscreenProgressState();
        progressState->showConfirmation = false;
        progressState->reservedFlag1 = true;
        progressState->reservedFlag2 = true;
        progressState->showTip = true;
        progressState->closeMode = 3;
        progressState->playerId = 0;
        progressState->backScene = EUIScene_None;
        params->m_completionData = progressState;

        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress,
                                             params, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);

        mSubRoundLoadStartTime = System::processTimeInMilliSecs();
        mLocalChunksLoaded = false;
        mServerIsWaitingOnChunkLoading = false;
        return;
    }
    default:
        return;
    }
}

void ClientMasterGameMode::OnExitedGame()
{
    CommonMasterGameMode::OnExitedGame();
    OnStateChange(eGameModeState_None);
}

void ClientMasterGameMode::ClearPlaylist()
{
    mPlaylist.clear();
    mVoteables->getVoteable(Voteables::eVoteable_Map)->clearVotes();
}

void ClientMasterGameMode::AddMapToPlaylist(int uniqueId, const std::wstring& name, int texturePackId)
{
    MapSelectInfo info;
    info.uniqueId = uniqueId;
    info.name = name;
    info.texturePackId = texturePackId;
    mPlaylist.push_back(info);
}

void ClientMasterGameMode::SetMapVote(int playerIndex, int vote, bool remove)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
    voteable->setVoteAndSync(playerIndex, vote, remove ? -1 : 1);
}

unsigned int ClientMasterGameMode::GetMapVote(int playerIndex, int vote)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
    return static_cast<unsigned int>(voteable->getVote(localPlayer, vote) >= 0);
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

void ClientMasterGameMode::GetCurrentMapVotesText(std::vector<HtmlString>& text)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    std::vector<MapSelectInfo> playlist;
    GetUniquePlaylist(playlist);

    const LevelRules::ELevelRuleType ruleType = LevelRules::GetRuleTypeForMiniGame(Minecraft::GetMiniGameId());

    for (std::vector<MapSelectInfo>::const_iterator it = playlist.begin(); it != playlist.end(); ++it)
    {
        const int uniqueId = it->uniqueId;
        LevelRules* levelRules = g_pApp->getLevelRules();
        LevelGenerationOptions* options = levelRules->getLevelGenFromUniqueId(ruleType, uniqueId);
        const wchar_t* worldName = options != NULL ? options->getWorldName() : L"Unknown";
        boost::unordered_map<int, int>::const_iterator vote = totals.find(uniqueId);
        if (g_pApp->DebugArtToolsOn())
        {
            const std::wstring label = vote == totals.end()
                                           ? formatwstr(L"%ls : 0", worldName)
                                           : formatwstr(L"%ls : %d", worldName, vote->second);
            text.push_back(HtmlString(label, eMinecraftColour_NOTSET, false, false, 0));
        }

        const unsigned int themeId = LevelGenerationOptions::GetThemeFromUniqueId(uniqueId);
        const unsigned int themeUniqueId = LevelGenerationOptions::CreateUniqueId(themeId, -1);
        vote = totals.find(static_cast<int>(themeUniqueId));
        const std::wstring themeLabel = vote == totals.end()
                                            ? formatwstr(L"%ls : 0", worldName)
                                            : formatwstr(L"%ls : %d", worldName, vote->second);
        text.push_back(HtmlString(themeLabel, eMinecraftColour_NOTSET, false, false, 0));
    }
}
void ClientMasterGameMode::GetCurrentVariantVotesText(std::vector<HtmlString>& text)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    MiniGameDef* definition =
        MiniGameDef::GetCustomGameModeById(GetNextGameMode(), true);
    int count = static_cast<int>(definition->GetGameVariantCount()) - 2;
    if (definition->IsCustomGame())
        count = definition->GetLootSetCount();
    for (int i = 0; i < count; ++i)
    {
        const std::wstring label = definition->GetLootSetText(i);
        boost::unordered_map<int, int>::const_iterator it = totals.find(i);
        if (it == totals.end())
            text.push_back(HtmlString(formatwstr(L"%ls : 0", label.c_str()),
                                      eMinecraftColour_NOTSET, false, false, 0));
        else
            text.push_back(HtmlString(formatwstr(L"%ls : %d", label.c_str(), it->second),
                                      eMinecraftColour_NOTSET, false, false, 0));
    }
}

void ClientMasterGameMode::SetKickVote(int playerIndex, unsigned char colour, bool vote)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
    if (!localPlayer)
        return;

    if (CGameNetworkManager::sInstance.GetPlayerBySmallId(colour) != NULL)
    {
        const unsigned int voteableId = g_pApp->GetPlayerColour(colour);
        Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Kick);
        voteable->setVoteAndSync(playerIndex, voteableId, vote);
    }
}

unsigned int ClientMasterGameMode::GetKickVote(int playerIndex, unsigned char colour)
{
    const unsigned int voteableId = g_pApp->GetPlayerColour(colour);
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Kick);
    return static_cast<unsigned int>(voteable->getVote(playerIndex, voteableId) >= 0);
}

void ClientMasterGameMode::SetBuildOffScoringVote(int playerIndex, int vote, int value)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring);
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
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring);
    return static_cast<unsigned int>(voteable->getVote(localPlayer, vote));
}

void ClientMasterGameMode::SetBuildOffThemeVote(int playerIndex, int vote, int value)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme);
    voteable->setVoteAndSync(playerIndex, vote, 1);
}

unsigned int ClientMasterGameMode::GetBuildOffThemeVote(int playerIndex)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme);
    return static_cast<unsigned int>(voteable->getVote(localPlayer, 0));
}

const wchar_t* ClientMasterGameMode::GetString(const int& id) { return mStringTable.getString(id); }
void ClientMasterGameMode::GetCurrentThemeVotesText(std::vector<HtmlString>& text)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    for (unsigned int index = 0; index < mThemeWordIds.size(); ++index)
    {
        const int themeId = mThemeWordIds[index];
        const wchar_t* name = GetString(themeId);
        const int voteCount = totals[themeId];
        const std::wstring label = formatwstr(L"%ls : %d", name, voteCount);
        text.push_back(HtmlString(label, eMinecraftColour_NOTSET, false, false, 0));
    }
}

std::wstring ClientMasterGameMode::GetTransitionTextOverride()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const unsigned int id = miniGame->GetId();

    if (id == 2)
    {
        if (miniGame->IsCustomGame())
            return miniGame->GetLootSetText(miniGame->GetLootSetId());
        return miniGame->GetLootSetText(miniGame->GetGameVariant());
    }

    if (id == 1 || id == 3)
    {
        if (mNextLevelGenOptions != NULL)
            return std::wstring(mNextLevelGenOptions->getWorldName());
        return std::wstring();
    }

    return std::wstring();
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
    mNextLevelGenOptions = options;
    if (owned)
        mNextLevelGenOptions->loadBaseSaveData();
    mActiveChunkAreas.clear();
    mNextLevelGenOptions->getRequiredGameRules()->getActiveChunkAreas(&mActiveChunkAreas);
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
                                   eMiniGameId_Any)) *
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
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) || tick != 3)
        return;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        if (gConsoleUIController.IsSceneInStack(playerIndex, EUIScene_GameModeScoreboardMenu))
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

void ClientMasterGameMode::OnChatPacket(boost::shared_ptr<ClientboundChatPacket> packet)
{
    if (!packet)
        return;
    const unsigned int messageId = static_cast<unsigned int>(packet->getMessage());
    const int tick = packet->getFirstInt();
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == NULL)
        return;
    if (messageId < 0x80U)
    {
        if (messageId == 0x7aU)
        {
            DoLobbyTickSound(tick, miniGame->getSoundSpec(EGameModeSound_RoundLimitTimer));
            return;
        }
        if (messageId != 0x7cU)
        {
            if (messageId != 0x7eU)
                return;
            DoRoundStartTickSound(tick, miniGame->getSoundSpec(EGameModeSound_RoundStartTimer));
            OnRoundStartTimerTick(tick);
            return;
        }
    }
    else
    {
        if (messageId < 0x81U)
        {
            DoLobbyTickSound(tick, miniGame->getSoundSpec(EGameModeSound_LobbyTimer));
            return;
        }
        if (messageId != 0x81U)
        {
            if (messageId != 0x86U)
                return;
            DoLobbyTickSound(tick, miniGame->getSoundSpec(EGameModeSound_RoundLimitTimer));
            return;
        }
    }
    DoGraceTickSound(packet->hasInts() ? tick : 0, miniGame->getSoundSpec(EGameModeSound_GraceTimer));
}
void ClientMasterGameMode::SetGeneratorSeed(long long seed, float scale)
{
    UpdateGeneratorSeed(seed, scale);
    ResetMapDistances();
}
bool ClientMasterGameMode::IsWaitingToStart() const { return GetGameModeState() == eGameModeState_WaitingToStart; }
bool ClientMasterGameMode::IsRoundStarting() const { return GetGameModeState() == eGameModeState_RoundStarting; }
bool ClientMasterGameMode::IsWaitingForMusicStartEvent() const
{
    if (!IsWaitingToStart() && !IsRoundStarting() && mGameModeState != eGameModeState_SelectingGameRules && mGameModeState != eGameModeState_LoadingRound)
        return false;
    return true;
}
bool ClientMasterGameMode::IsRoundEnding() const { return GetGameModeState() == eGameModeState_RoundEnding; }
bool ClientMasterGameMode::IsRoundPlaying() const { return GetGameModeState() == eGameModeState_RoundPlaying; }
bool ClientMasterGameMode::IsInScoringState() const
{
    return Minecraft::InMiniGame(eMiniGameId_Build, true) && GetGameModeState() == eGameModeState_PostRound;
}
bool ClientMasterGameMode::IsInLobbyState() { return GetGameModeState() <= eGameModeState_LoadingRound; }
bool ClientMasterGameMode::IsInActiveRoundState() { return GetGameModeState() >= eGameModeState_RoundStarting && GetGameModeState() <= eGameModeState_LoadingSubRound; }
bool ClientMasterGameMode::IsInTrasitionFromLobby()
{
    const EGameModeState state = mGameModeState;
    return state == eGameModeState_None || state == eGameModeState_SelectingGameRules || state == eGameModeState_LoadingRound;
}
bool ClientMasterGameMode::CheckLocalChunksLoaded()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Level* level = minecraft->getLevel(0);
    bool allLoaded = true;

    mLoadedChunkCount = 0;
    for (std::vector<AABB*>::const_iterator areaIt = mActiveChunkAreas.begin();
         areaIt != mActiveChunkAreas.end() && allLoaded; ++areaIt)
    {
        const AABB* area = *areaIt;
        if (area == NULL)
            continue;

        const int minChunkX = static_cast<int>(area->mMinX);
        const int maxChunkX = static_cast<int>(area->mMaxX);
        const int minChunkZ = static_cast<int>(area->mMinZ);
        const int maxChunkZ = static_cast<int>(area->mMaxZ);

        for (int chunkX = minChunkX; chunkX <= maxChunkX && allLoaded; ++chunkX)
        {
            for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
            {
                if (!level->reallyHasChunk(chunkX, chunkZ))
                {
                    allLoaded = false;
                    break;
                }
                ++mLoadedChunkCount;
            }
        }
    }

    if (!allLoaded || mBRUPsRemaining >= 1)
        return false;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
        ClientPacketListener* connection = minecraft->getConnection(playerIndex);
        if (localPlayer != NULL && connection != NULL)
        {
            GameModePacket::EmptyData emptyData;
            connection->send(GameModePacket::Create(GameModePacket::eMessage_FinishedLoadingLocally, emptyData));
        }
    }

    mServerIsWaitingOnChunkLoading = false;
    return true;
}
void ClientMasterGameMode::FireGameRoundStats()
{
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
        return;
    LevelGenerationOptions* options = g_pApp != NULL ? g_pApp->getLevelGenerationOptions() : NULL;
    if (options == NULL)
        options = mNextLevelGenOptions;
    const MiniGameDef* current = GetMiniGame();
    if (current == NULL)
        return;
    MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
        current->GetId(), true);
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
void ClientMasterGameMode::updateProgressRenderer()
{
    ProgressRenderer* renderer = Minecraft::GetInstance()->mProgressRenderer;
    if (renderer == NULL)
        return;
    const MiniGameDef* miniGame = GetMiniGame();
    if (miniGame == NULL)
        return;
    if (miniGame->GetId() == 1)
    {
        int progress = 0;
        if (!mActiveChunkAreas.empty())
        {
            const AABB* area = mActiveChunkAreas.at(0);
            const int area2D = (static_cast<int>(area->mMaxX) - static_cast<int>(area->mMinX)) *
                               (static_cast<int>(area->mMaxZ) - static_cast<int>(area->mMinZ));
            if (area2D > 0)
            {
                progress = (mLoadedChunkCount * 100) / area2D;
                if (progress > 99)
                {
                    renderer->progressStage(0xd56bcda0);
                    renderer->progressStagePercentage(99);
                    return;
                }
            }
        }
        renderer->progressStage(0xebb9a246);
        renderer->progressStagePercentage(progress);
        return;
    }
    if (miniGame->GetId() != 2)
        return;
    const int layerCount = static_cast<int>(GetLayerCount());
    if (mSubRoundLoadStartTime == 0x7fffffff)
    {
        renderer->progressStage(0xd5ac1bc8);
        renderer->progressStagePercentage(0);
        return;
    }
    const int elapsed = static_cast<int>(System::processTimeInMilliSecs() - mSubRoundLoadStartTime);
    const int progress = static_cast<int>(static_cast<unsigned int>(elapsed * 100) /
                                          static_cast<unsigned int>(layerCount * 1500 + 1500));
    renderer->progressStage(progress < 100 ? 0xd5ac1bc8 : 0xebb9a246);
    float percent = static_cast<float>(progress);
    if (percent < 0.0f)
        percent = 0.0f;
    else if (percent > 100.0f)
        percent = 100.0f;
    renderer->progressStagePercentage(Mth::floor((percent / 100.0f) * 80.0f));
}
void ClientMasterGameMode::Tick()
{
    CommonMasterGameMode::Tick();

    if (mNewLevelDataPending && mNextLevelGenOptions != NULL &&
        mNextLevelGenOptions->hasBaseSaveData())
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        MultiPlayerLevel* level = static_cast<MultiPlayerLevel*>(minecraft->getLevel(0));
        level->BlockUntilSafeToChangeSaveData(false);
        level->refreshFromLevelGen();
        gRegionFileCache._clear();
        mNewLevelDataPending = false;
    }

    if (mServerIsWaitingOnChunkLoading && CheckLocalChunksLoaded())
        mLocalChunksLoaded = true;

    switch (mGameModeState)
    {
    case eGameModeState_RoundPlaying:
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        CheckProgress(minecraft->mLevel->mPlayers);
        CheckCheckpoints(minecraft->mLevel->mPlayers);

        GhostController* ghosts = minecraft->GetGhostController();
        if (ghosts != NULL)
        {
            const std::vector<boost::shared_ptr<Player> > ghostPlayers = ghosts->getPlayers();
            CheckCheckpoints(ghostPlayers);
            CheckProgress(ghostPlayers);

            const std::vector<boost::shared_ptr<GhostPlayer> > ghostEntities =
                ghosts->getGhostPlayers();
            for (std::vector<boost::shared_ptr<GhostPlayer> >::const_iterator it =
                     ghostEntities.begin();
                 it != ghostEntities.end(); ++it)
            {
                const boost::shared_ptr<GhostPlayer>& ghost = *it;
                if (!ghost)
                    continue;
                ghost->mXOld = ghost->mX;
                ghost->mYOld = ghost->mY;
                ghost->mZOld = ghost->mZ;
            }
        }
        FireGameRoundStats();
        break;
    }
    case eGameModeState_PostRound:
        FireGameRoundStats();
        break;
    case eGameModeState_RoundEnding:
    {
        FireGameRoundStats();
        GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
        if (ghosts != NULL)
            ghosts->stopPlayback();
        break;
    }
    case eGameModeState_LoadingSubRound:
        updateProgressRenderer();
        break;
    default:
        break;
    }
}
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
    Minecraft* minecraft = Minecraft::GetInstance();

    if (transition->mReturningToLobby)
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
            if (localPlayer != NULL)
            {
                localPlayer->clientSideCloseContainer();
                localPlayer->mHasOpenedInventory = false;
            }
        }
    }

    minecraft->mGameRenderer->DisableUpdateThread(1);
    minecraft->mLevel->DropChunksOutsideArea(transition->mMinChunkX, transition->mMinChunkZ,
                                              transition->mMaxChunkX, transition->mMaxChunkZ, true);
    minecraft->mGameRenderer->EnableUpdateThread(1);
    mTransitions.push_back(transition);
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
    mActiveChunkAreas.clear();
    for (std::vector<TargetAreaRuleDefinition*>::const_iterator it = targetAreas.begin();
         it != targetAreas.end(); ++it)
    {
        if (*it != NULL && (*it)->mPrimary)
            mActiveChunkAreas.push_back((*it)->getArea());
    }
}
void ClientMasterGameMode::SetServerIsWaitingOnChunkLoading() { mServerIsWaitingOnChunkLoading = true; }
bool ClientMasterGameMode::ServerIsWaitingOnUsLoading() { return mServerIsWaitingOnChunkLoading; }
void ClientMasterGameMode::RecievedBRUP() { if (mBRUPsRemaining > 0) --mBRUPsRemaining; }
void ClientMasterGameMode::SetBRUPSToWaitOn(int count) { mBRUPCount = count; mBRUPsRemaining = count; }
bool ClientMasterGameMode::IsValidStateForItemUse() const
{
    if (IsWaitingToStart())
        return false;
    if (IsRoundStarting())
        return false;
    if (IsRoundRestarting())
        return false;
    return mGameModeState != eGameModeState_LoadingRound;
}
EGameModeState ClientMasterGameMode::GetGameModeState() const { return mGameModeState; }
MultiPlayerLevel* ClientMasterGameMode::GeneratorTargetLevel() const { return Minecraft::GetInstance()->mLevels[0]; }
Scoreboard* ClientMasterGameMode::getScoreboard() { return Minecraft::GetInstance()->mLevel->GetClientScoreboard(); }

double ClientMasterGameMode::GetPlayerMapProgress(PlayerUID uid) const
{
    boost::unordered_map<PlayerUID, double, boost::hash<PlayerUID> >::const_iterator it = mPlayerProgress.find(uid);
    return it == mPlayerProgress.end() ? 0.0 : it->second;
}

void ClientMasterGameMode::OnProgressMade(const boost::shared_ptr<Player>& player, double progress)
{
    if (player->GetType() != EntityType::GHOST_PLAYER)
        mPlayerProgress[player->getSUID().GetID()] = progress;
}

void ClientMasterGameMode::ResetMapDistances() { CommonMasterGameMode::ResetMapDistances(); mPlayerProgress.clear(); mLastCheckpoints.clear(); }
int ClientMasterGameMode::convertCheckpointIDToSpawnableID(int checkpoint)
{
    int spawnableIndex = 0;
    int lastSequenceId = -1;

    if (g_pApp == NULL)
        return spawnableIndex;

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options == NULL)
        return spawnableIndex;

    LevelRuleset* rules = options->getRequiredGameRules();
    if (rules == NULL)
        return spawnableIndex;

    std::vector<CheckpointRuleDefinition*>* checkpoints = rules->getCheckpoints();
    if (checkpoints == NULL)
        return spawnableIndex;

    int checkpointIndex = 0;
    for (std::vector<CheckpointRuleDefinition*>::const_iterator it = checkpoints->begin();
         it != checkpoints->end(); ++it, ++checkpointIndex)
    {
        CheckpointRuleDefinition* rule = *it;
        if (rule != NULL && rule->hasSpawnPoint())
        {
            if (checkpointIndex == checkpoint)
            {
                const int sequenceId = rule->getSequenceID();
                if (lastSequenceId == sequenceId)
                    return spawnableIndex - 2;
                break;
            }

            const int sequenceId = rule->getSequenceID();
            if (lastSequenceId != sequenceId)
            {
                ++spawnableIndex;
                lastSequenceId = sequenceId;
            }
        }
    }

    return spawnableIndex;
}
void ClientMasterGameMode::SetLocalRoundTimeStart(long long value)
{
    mLocalRoundTimeStart = System::processTimeInMilliSecs() - value;
    if (value != 0)
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    GhostController* ghosts = minecraft->GetGhostController();
    if (ghosts == NULL)
        return;

    int ghostSlot = 0;
    for (int pair = 0; pair < 2; ++pair)
    {
        for (int sub = 0; sub < 2; ++sub)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                minecraft->mLocalPlayers[pair * 2 + sub];
            boost::shared_ptr<Player> player = localPlayer;
            if (player != NULL && !player->isSpectator())
            {
                if (ghosts->mRecordingActive[ghostSlot])
                    ghosts->stopRecording(ghostSlot);
                Vec3* pos = Vec3::newTemp(player->mX, player->mY, player->mZ);
                ghosts->startRecording(ghostSlot, pos);
            }
            ++ghostSlot;
        }
    }

    const MiniGameDef* miniGame = CommonMasterGameMode::GetMiniGame();
    if (miniGame != NULL && miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        ghosts->startPlayback();
}
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
