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
#include "game/core/CConsoleMinecraftApp.hpp"
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

bool ClientMasterGameMode::IsRoundRestarting() const { return mGameModeState == eGameModeState_LoadingSubRound; }

void ClientMasterGameMode::ResetReadyStates()
{
    std::memset(mPlayerReady, 0, sizeof(mPlayerReady));
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

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && scoreboard->mCurrentRound > 0)
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
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

    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    Minecraft::GetInstance()->mSoundEngine->FlagMusicEventStart();

    switch (GetMiniGame()->GetId())
    {
    case 1:
    case 4:
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG01_GAME_START, sound.Volume(), 1.0f);
        break;
    case 2:
        miniGame->SetDisabledEditActions(false);
        static_cast<ConsoleSoundEngine*>(Minecraft::GetInstance()->mSoundEngine)->RemoveMusicVolumeLimit();
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG02_GAME_START, sound.Volume(), 1.0f);
        break;
    case 3:
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG03_COUNTDOWN_ZERO, sound.Volume(), 1.0f);

        if (Minecraft::GetInstance()->GetGhostController() != NULL)
        {
            for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player =
                    Minecraft::GetInstance()->mLocalPlayers[playerIndex];
                if (player != NULL && !player->isSpectator())
                    Minecraft::GetInstance()->GetGhostController()->startRecording(playerIndex, player->getPos(1.0f));
            }
            if (GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
                Minecraft::GetInstance()->GetGhostController()->startPlayback();
        }
        break;
    }
}

void ClientMasterGameMode::OnRoundEnding()
{
    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    switch (GetMiniGame()->GetId())
    {
    case 1:
    case 4:
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundEnding);
        PlaySound(SoundEvents::MG01_CELEBRATION, sound.Volume(), 1.0f);
        break;
    case 2:
        miniGame->SetDisabledEditActions(false);
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundEnding);
        PlaySound(SoundEvents::MG02_GAME_END, sound.Volume(), 1.0f);
        break;
    }

    ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        if (player != NULL)
        {
            const StatsUID suid = player->getSUID();
            if (!scoreboard->IsWinner(suid, -1))
            {
                gConsoleUIController.CloseUIScenes(playerIndex, true);
                if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
                    mStrings[player->getName()] = 0;
            }
            else if (Minecraft::InMiniGame(eMiniGameId_Battle, true) &&
                     CGameNetworkManager::sInstance.IsInPublicJoinableGame())
            {
                player->awardStat(GenericStats::theStudent(), GenericStats::param_theStudent());
                const unsigned int winCount = ++mStrings[player->getName()];
                if (winCount > 2)
                    player->awardStat(GenericStats::hasBecomeTheMaster(),
                                      GenericStats::param_hasBecomeTheMaster());
            }
        }
    }

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
    info.name.assign(name);
    info.texturePackId = texturePackId;
    mPlaylist.push_back(info);
}

void ClientMasterGameMode::SetMapVote(int playerIndex, int vote, bool remove)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    mVoteables->getVoteable(Voteables::eVoteable_Map)->setVoteAndSync(localPlayer->mPlayerIndex, vote, remove ? -1 : 1);
}

unsigned int ClientMasterGameMode::GetMapVote(int playerIndex, int vote)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_Map)->getVote(minecraft->mLocalPlayers[playerIndex], vote) >= 0);
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
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_Kick)->getVote(localPlayer->mPlayerIndex, gApp.GetPlayerColour(colour)) >= 0);
}

void ClientMasterGameMode::SetBuildOffScoringVote(int playerIndex, int vote, int value)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring)->setVoteAndSync(localPlayer->mPlayerIndex, vote, static_cast<char>(value));
}

unsigned int ClientMasterGameMode::GetBuildOffTeamIndexToScore(int playerIndex)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    if (player != NULL)
    {
        Team* team = GetTeamAtPosition(player->mX, player->mY, player->mZ, 6.0);
        if (team != NULL && team != player->getTeam())
            return team->getTeamIndex();
    }

    return 0xffffffffU;
}

unsigned int ClientMasterGameMode::GetBuildOffScoringVote(int playerIndex, int vote)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring)->getVote(minecraft->mLocalPlayers[playerIndex], vote));
}

void ClientMasterGameMode::SetBuildOffThemeVote(int playerIndex, int vote, int value)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme)->setVoteAndSync(localPlayer->mPlayerIndex, vote, 1);
}

unsigned int ClientMasterGameMode::GetBuildOffThemeVote(int playerIndex)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme)->getVote(minecraft->mLocalPlayers[playerIndex], 0));
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
    MiniGameDef::SoundSpec sound;
    switch (GetMiniGame()->GetId())
    {
    case 1:
        sound = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(EGameModeSound_OnShowdown);
        PlaySound(SoundEvents::MG01_SHOWDOWN, sound.Volume(), 1.0f);
        break;
    case 2:
    case 4:
        sound = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(EGameModeSound_OnShowdown);
        PlaySound(SoundEvents::MG02_SHOWDOWN, sound.Volume(), 1.0f);
        break;
    }
}
void ClientMasterGameMode::OnChestRefill()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnChestRefill);
    PlaySound(SoundEvents::MG01_CHEST, static_cast<float>(sound.Volume()), 1.0f);
}
void ClientMasterGameMode::OnDeath(const Vec3* position)
{
    const float pitchStep =
        static_cast<float>(8 - Minecraft::GetInstance()->mLevel->GetClientScoreboard()->GetActivePlayerCount(
                                   eMiniGameId_Any)) *
        0.125f * 0.25f;
    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    switch (GetMiniGame()->GetId())
    {
    case 1:
        sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);
        PlaySound(SoundEvents::MG01_DEATH, sound.Volume() - pitchStep, 1.0f - pitchStep);
        break;
    case 2:
        miniGame->SetDisabledEditActions(false);
        sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);
        PlaySound(SoundEvents::MG02_DEATH, sound.Volume(), 1.0f);
        break;
    case 3:
        if (position != NULL)
        {
            sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);
            Minecraft::GetInstance()->mSoundEngine->play(
                SoundEvents::MG03_DEATH, static_cast<float>(position->x),
                static_cast<float>(position->y), static_cast<float>(position->z),
                sound.Volume(), 1.0f, NULL);
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
    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) || scoreboard->mCurrentRound < 0 || tick != 3)
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
bool ClientMasterGameMode::IsWaitingToStart() const { return mGameModeState == eGameModeState_WaitingToStart; }
bool ClientMasterGameMode::IsRoundStarting() const { return mGameModeState == eGameModeState_RoundStarting; }
bool ClientMasterGameMode::IsWaitingForMusicStartEvent() const
{
    if (!IsWaitingToStart() && !IsRoundStarting() && mGameModeState != eGameModeState_SelectingGameRules && mGameModeState != eGameModeState_LoadingRound)
        return false;
    return true;
}
bool ClientMasterGameMode::IsRoundEnding() const { return mGameModeState == eGameModeState_RoundEnding; }
bool ClientMasterGameMode::IsRoundPlaying() const { return mGameModeState == eGameModeState_RoundPlaying; }
bool ClientMasterGameMode::IsInScoringState() const
{
    return Minecraft::InMiniGame(eMiniGameId_Build, true) && mGameModeState == eGameModeState_PostRound;
}
int ClientMasterGameMode::IsInLobbyState()
{
    int inLobby = 0;
    if (static_cast<unsigned int>(mGameModeState) <= eGameModeState_LoadingRound)
        inLobby = 1;
    return inLobby;
}
int ClientMasterGameMode::IsInActiveRoundState()
{
    unsigned int state = mGameModeState;
    if (state < eGameModeState_RoundStarting)
        return 0;
    if (state > eGameModeState_LoadingSubRound)
        return 0;
    return 1;
}
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
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        if (mCurrentLevelGenOptions != NULL && options == NULL)
            options = mCurrentLevelGenOptions;

        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(GetMiniGame()->GetId(), true);
        if (definition->IsCRCValid() && options != NULL && options->isCRCValid())
        {
            ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
            GetMiniGame()->GetGameVariant();
            for (int slot = 0; slot < 4; ++slot)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[slot];
                if (player != NULL && !player->isSpectator())
                {
                    GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
                    const MiniGameDef* miniGame = GetMiniGame();
                    if (ghosts != NULL)
                    {
                        if (scoreboard->GetEndZoneEntered(player->getSUID(), -1) && ghosts->mRecordingPending[slot])
                        {
                            int recorded;
                            if (miniGame->ArePointsActive())
                            {
                                recorded = ghosts->RecordScore(slot, options->getUniqueId(),
                                                               scoreboard->GetPoints(player->getSUID(), -1),
                                                               scoreboard->GetLifeSpan(player->getSUID(), -1));
                            }
                            else
                            {
                                recorded = ghosts->RecordTime(slot, options->getUniqueId(),
                                                              scoreboard->GetLifeSpan(player->getSUID(), -1));
                            }
                            if (recorded != 0)
                                scoreboard->SetPersonalBest(player->getSUID(), -1);
                        }
                    }
                }
            }
        }
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

    if (mNextLevelGenOptions != NULL && mNewLevelDataPending && mNextLevelGenOptions->hasBaseSaveData())
    {
        Minecraft::GetInstance()->getLevel(0)->BlockUntilSafeToChangeSaveData(false);
        Minecraft::GetInstance()->getLevel(0)->refreshFromLevelGen();
        gRegionFileCache._clear();
        mCurrentLevelGenOptions = mNextLevelGenOptions;
        mNextLevelGenOptions = NULL;
        mNewLevelDataPending = false;
    }

    if (mServerIsWaitingOnChunkLoading && CheckLocalChunksLoaded())
        mLocalChunksLoaded = true;

    switch (mGameModeState)
    {
    case eGameModeState_RoundPlaying:
    {
        CheckProgress(Minecraft::GetInstance()->getLevel(0)->mPlayers);
        CheckCheckpoints(Minecraft::GetInstance()->getLevel(0)->mPlayers);

        GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
        if (ghosts != NULL)
        {
            const std::vector<boost::shared_ptr<Player> > ghostPlayers = ghosts->getPlayers();
            CheckCheckpoints(ghostPlayers);
            CheckProgress(ghostPlayers);

            const std::vector<boost::shared_ptr<GhostPlayer> > ghostEntities = ghosts->getGhostPlayers();
            for (std::vector<boost::shared_ptr<GhostPlayer> >::const_iterator it = ghostEntities.begin();
                 it != ghostEntities.end(); ++it)
            {
                boost::shared_ptr<GhostPlayer> ghost = *it;
                ghost->mXOld = ghost->mX;
                ghost->mYOld = ghost->mY;
                ghost->mZOld = ghost->mZ;
            }
        }
    }
    case eGameModeState_PostRound:
    case eGameModeState_RoundEnding:
        FireGameRoundStats();
        if (Minecraft::GetInstance()->GetGhostController() != NULL && mGameModeState == eGameModeState_RoundEnding)
            Minecraft::GetInstance()->GetGhostController()->stopPlayback();
        break;
    case eGameModeState_LoadingSubRound:
        updateProgressRenderer();
        break;
    default:
        break;
    }
}
bool ClientMasterGameMode::isNewLevelDataPending() { return mNewLevelDataPending; }
bool ClientMasterGameMode::isWaitingToTransition() { return !mTransitions.empty() && gApp.GetGameStarted() && gApp.GetXuiAction(gProfile->GetPrimaryPad()) == eXuiAction_Idle; }

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
    while (!mTransitions.empty())
        delete popTransition();
}

void ClientMasterGameMode::onChangeCustomGameMode()
{
    if (gApp.getLevelGenerationOptions() == NULL)
        return;

    LevelRuleset* rules = gApp.getLevelGenerationOptions()->getRequiredGameRules();
    setCheckpoints(*rules->getCheckpoints());

    std::vector<TargetAreaRuleDefinition*> targetAreas;
    rules->getTargetAreas(&targetAreas);
    mPrimaryTargetAreas.clear();
    for (std::vector<TargetAreaRuleDefinition*>::const_iterator it = targetAreas.begin();
         it != targetAreas.end(); ++it)
    {
        if ((*it)->mPrimary)
            mPrimaryTargetAreas.push_back((*it)->getArea());
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

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        LevelRuleset* rules = options->getRequiredGameRules();
        std::vector<CheckpointRuleDefinition*> checkpoints = *rules->getCheckpoints();
        int lastSequenceId = -1;
        int checkpointIndex = 0;
        for (std::vector<CheckpointRuleDefinition*>::const_iterator it = checkpoints.begin();
             it != checkpoints.end(); ++it, ++checkpointIndex)
        {
            if ((*it)->hasSpawnPoint())
            {
                if (checkpointIndex == checkpoint)
                {
                    --spawnableIndex;
                    if (lastSequenceId == (*it)->getSequenceID())
                        --spawnableIndex;
                    break;
                }

                if (lastSequenceId != (*it)->getSequenceID())
                {
                    ++spawnableIndex;
                    lastSequenceId = (*it)->getSequenceID();
                }
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
