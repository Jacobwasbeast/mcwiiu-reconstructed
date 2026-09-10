#include "game/network/CGameNetworkManager.hpp"
#include "game/core/Socket.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Gui.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/core/GameSessionData.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"
#include "platform/wiiu/cCommerceNintendo.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/Block.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/network/DisconnectPacket.hpp"

extern "C" int shutdown(int socket, int how);
extern GameRuleManager g_GameRuleManager;
extern ConsoleUIController gConsoleUIController;
extern "C" int close(int socket);
void SetThreadName(unsigned int threadId, const char* name);

CGameNetworkManager CGameNetworkManager::sInstance;

CGameNetworkManager::CGameNetworkManager()
    : m_serverStoppedEvent(0),
      m_serverReadyEvent(0),
      m_platform(0),
      m_pendingInviteUserIndex(-1),
      m_pendingInviteUserMask(0),
      m_pendingInvitePresence(0),
      m_initialised(false),
      m_networkThreadRunning(false),
      m_disconnectError(false),
      m_unknownState(false),
      m_publicJoinable(false),
      m_inviteInfo(0)
{
    InitializeCriticalSection(&m_connectionsMutex);
}
CGameNetworkManager::~CGameNetworkManager() { DeleteCriticalSection(&m_connectionsMutex); }

INetworkPlayer* CGameNetworkManager::GetLocalPlayerByUserIndex(int userIndex) { return m_platform ? m_platform->GetLocalPlayerByUserIndex(userIndex) : 0; }
void CGameNetworkManager::ServerStoppedCreate(bool create) { m_serverStoppedEvent = create ? new C4JEventImpl(C4JEvent::Mode0) : 0; }
void CGameNetworkManager::ServerReadyCreate(bool create) { m_serverReadyEvent = create ? new C4JEventImpl(C4JEvent::Mode0) : 0; }
void CGameNetworkManager::Initialise()
{
    ServerStoppedCreate(false);
    ServerReadyCreate(false);
    m_platform = CPlatformNetworkManager::Create();
    m_platform->Initialise(this, LevelRenderer::getGlobalChunkCount() / 16);
    m_networkThreadRunning = false;
    m_initialised = true;
}
void CGameNetworkManager::Terminate() { if (m_initialised) m_platform->Terminate(); }
int CGameNetworkManager::GetLocalPlayerMask(int userIndex) { return sInstance.m_platform ? sInstance.m_platform->GetLocalPlayerMask(userIndex) : 0; }
bool CGameNetworkManager::IsInSession() { return m_platform && m_platform->IsInSession(); }
bool CGameNetworkManager::JoinGameFromInviteInfo(int userIndex, int mask, const FriendPresence* presence)
{
    bool joining = m_platform->JoinGameFromInviteInfo(userIndex, mask, presence);
    if (joining)
    {
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(true);
    }
    return joining;
}
bool CGameNetworkManager::SetLocalGame(bool localGame) { return m_platform->SetLocalGame(localGame); }
int CGameNetworkManager::JoinFromInvite_SignInReturned(void*, bool result, int error) { return result ? 1 : error; }
bool CGameNetworkManager::HandleInviteWhenInMenus(int userIndex, const FriendPresence* presence) { return JoinGameFromInviteInfo(userIndex, 0, presence); }
void CGameNetworkManager::ProcessInvite(unsigned int userIndex, unsigned int flags, FriendPresence* presence)
{
    m_pendingInvitePresence = presence;
    m_pendingInviteUserIndex = (int)userIndex;
    m_pendingInviteUserMask = flags;
    gApp.SetAction((int)userIndex, (eXuiAction)0x14, 0);
}
void CGameNetworkManager::GameInviteReceived(int userIndex, FriendPresence* presence) { HandleInviteWhenInMenus(userIndex, presence); }
bool CGameNetworkManager::TickInvitePending()
{
    if (m_pendingInviteUserIndex >= 0 && m_pendingInvitePresence != 0 &&
        !gConsoleUIController.IsShowingErrorMessage(m_pendingInviteUserIndex))
    {
        GameInviteReceived(m_pendingInviteUserIndex, m_pendingInvitePresence);
        m_pendingInviteUserIndex = -1;
        return true;
    }
    return false;
}
void CGameNetworkManager::DoWork() { if (m_platform) m_platform->DoWork(); TickInvitePending(); }
bool CGameNetworkManager::IsHost() { return m_platform && m_platform->IsHost(); }
void* CGameNetworkManager::ServerThreadProc(void* data)
{
    unsigned int* parameters = static_cast<unsigned int*>(data);
    unsigned int seed = 0;
    void* argument = 0;
    if (parameters != 0)
    {
        seed = parameters[0];
        argument = reinterpret_cast<void*>(parameters[1]);
        gApp.SetGameHostOption(static_cast<eGameHostOption>(0x20), parameters[3]);
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        miniGame->SetHostOption(static_cast<eCustomGameModeHostOption>(10), parameters[4]);
        if (parameters[5] != 0 &&
            gApp.getLevelGenerationOptions()->getParentPackId() != parameters[6])
        {
            while (Minecraft::GetInstance()->mTexturePackRepository->needsUIUpdate() ||
                   gConsoleUIController.IsReloadingSkin())
                C4JThread::Sleep(1);
            gApp.getLevelGenerationOptions()->loadBaseSaveData();
        }
    }
    SetThreadName(0xffffffff, "Minecraft Server thread");
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Compression::UseDefaultThreadStorage();
    OldChunkStorage::UseDefaultThreadStorage();
    Entity::useSmallIds();
    Level::enableLightingCache();
    Block::CreateNewThreadStorage();
    FireworksRecipe::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(static_cast<ElementStorage::eHESThreadId>(1));
    LevelChunk::CreateNewThreadStorage();
    MinecraftServer::main(seed, argument);
    AABB::ReleaseThreadStorage();
    Vec3::ReleaseThreadStorage();
    phys::Vec2::ReleaseThreadStorage();
    IntCache::ReleaseThreadStorage();
    Level::destroyLightingCache();
    Block::ReleaseThreadStorage();
    FireworksRecipe::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    LevelChunk::ReleaseThreadStorage();
    if (data != 0)
        free(data);
    return 0;
}
void CGameNetworkManager::ServerReadyWait() { if (m_serverReadyEvent) m_serverReadyEvent->WaitForSignal(-1); }
void CGameNetworkManager::ServerReadyDestroy() { delete m_serverReadyEvent; m_serverReadyEvent = 0; }
bool CGameNetworkManager::IsReadyToPlayOrIdle() { return m_platform && m_platform->IsReadyToPlayOrIdle(); }
int CGameNetworkManager::CorrectErrorIDS(int ids) { return m_platform ? m_platform->CorrectErrorIDS(ids) : ids; }
int CGameNetworkManager::GetJoiningReadyPercentage() { return m_platform ? m_platform->GetJoiningReadyPercentage() : 0; }
bool CGameNetworkManager::IsLeavingGame() { return m_platform && m_platform->IsLeavingGame(); }
int CGameNetworkManager::GetPlayerCount() { return m_platform ? m_platform->GetPlayerCount() : 0; }
bool CGameNetworkManager::IsLocalGame() { return m_platform->IsLocalGame(); }
bool CGameNetworkManager::StartNetworkGame(Minecraft*, void*) { return _RunNetworkGame(0); }
bool CGameNetworkManager::_RunNetworkGame(void*) { return m_platform != 0; }
int CGameNetworkManager::GetOnlinePlayerCount() { return m_platform ? m_platform->GetOnlinePlayerCount() : 0; }
bool CGameNetworkManager::AddLocalPlayerByUserIndex(int index) { return m_platform && m_platform->AddLocalPlayerByUserIndex(index); }
bool CGameNetworkManager::RemoveLocalPlayerByUserIndex(int index) { return m_platform && m_platform->RemoveLocalPlayerByUserIndex(index); }
INetworkPlayer* CGameNetworkManager::GetPlayerByIndex(int index) { return m_platform ? m_platform->GetPlayerByIndex(index) : 0; }
INetworkPlayer* CGameNetworkManager::GetPlayerByXuid(PlayerUID uid) { return m_platform ? m_platform->GetPlayerByXuid(uid) : 0; }
INetworkPlayer* CGameNetworkManager::GetPlayerBySmallId(unsigned char id) { return m_platform ? m_platform->GetPlayerBySmallId(id) : 0; }
std::wstring CGameNetworkManager::GetDisplayNameByGamertag(std::wstring name) { return m_platform ? m_platform->GetDisplayNameByGamertag(name) : name; }
INetworkPlayer* CGameNetworkManager::GetHostPlayer() { return m_platform ? m_platform->GetHostPlayer() : 0; }
void CGameNetworkManager::RegisterPlayerChangedCallback(int i, void (*cb)(void*, INetworkPlayer*, bool), void* d)
{
    if (m_platform)
    {
        m_platform->RegisterPlayerChangedCallback(i, cb, d);
    }
}
void CGameNetworkManager::UnRegisterPlayerChangedCallback(int i, void (*cb)(void*, INetworkPlayer*, bool), void* d)
{
    if (m_platform)
    {
        m_platform->UnRegisterPlayerChangedCallback(i, cb, d);
    }
}
void CGameNetworkManager::HandleSignInChange() { if (m_platform) m_platform->HandleSignInChange(); }
bool CGameNetworkManager::ShouldMessageForFullSession() { return m_platform && m_platform->ShouldMessageForFullSession(); }
bool CGameNetworkManager::IsInGameplay() { return m_platform && m_platform->IsInGameplay(); }
void CGameNetworkManager::SetPrivateGame(bool value) { m_platform->SetPrivateGame(value); }
bool CGameNetworkManager::IsPrivateGame() { return m_platform->IsPrivateGame(); }
void CGameNetworkManager::HostGame(int a, bool b, bool c, unsigned char d, EMiniGameId e, unsigned int f, unsigned int* g) { if (m_platform) m_platform->HostGame(a,b,c,d,e,f,g); }
bool CGameNetworkManager::IsInStatsEnabledSession() { return m_platform && m_platform->IsInStatsEnabledSession(); }
bool CGameNetworkManager::IsMatchmakingEnabled() { return m_platform && m_platform->IsMatchmakingEnabled(); }
bool CGameNetworkManager::IsInPublicJoinableGame() { return (m_publicJoinable || IsMatchmakingEnabled()) && !IsLocalGame(); }
void CGameNetworkManager::SetIsInPublicJoinableGame(bool value) { m_publicJoinable = value; }
bool CGameNetworkManager::IsInActiveCoopOrCompetitive(int)
{
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (gApp.GetGameHostOption((eGameHostOption)0x1f) &&
        (!mode->IsInActiveRoundState() || mode->IsRoundEnding() ||
         mode->IsRoundStarting() || mode->IsRoundRestarting()))
    {
        return false;
    }
    return true;
}
bool CGameNetworkManager::SessionHasSpace(unsigned int n) { return m_platform && m_platform->SessionHasSpace(n); }
bool CGameNetworkManager::SessionHasMii(int n) { return m_platform && m_platform->SessionHasMii(n); }
int CGameNetworkManager::GetSessionList(int a, int b, bool c) { return m_platform ? m_platform->GetSessionList(a,b,c) : 0; }
bool CGameNetworkManager::GetGameSessionInfo(int a, unsigned int b, FriendSessionInfo* c) { return m_platform && m_platform->GetGameSessionInfo(a,b,c); }
void CGameNetworkManager::SetSessionsUpdatedCallback(void (*cb)(void*), void* d) { if (m_platform) m_platform->SetSessionsUpdatedCallback(cb,d); }
void CGameNetworkManager::GetFullFriendSessionInfo(FriendSessionInfo* a, void (*cb)(bool,void*), void* d) { if (m_platform) m_platform->GetFullFriendSessionInfo(a,cb,d); }
void CGameNetworkManager::ForceFriendsSessionRefresh() { if (m_platform) m_platform->ForceFriendsSessionRefresh(); }
void CGameNetworkManager::ClearFriendsSessions() { if (m_platform) m_platform->ClearFriendsSessions(); }
int CGameNetworkManager::JoinGame(FriendSessionInfo* info, int userIndex, bool matchmaking)
{
    if (m_platform == 0)
    {
        return 0;
    }
    return m_platform->JoinGame(info, userIndex, 0, matchmaking);
}
bool CGameNetworkManager::FindAndJoinGame(int a, EMiniGameId b, unsigned int c, unsigned int* d, unsigned int* e)
{
    return m_platform && m_platform->FindAndJoinGame(a, 0, b, c, d, e);
}
void CGameNetworkManager::CancelJoinGame(void*) {}
bool CGameNetworkManager::LeaveGame(bool e) { Minecraft::GetInstance()->mGui->clearMessages(-1); return m_platform->LeaveGame(e); }
void CGameNetworkManager::UpdateAndSetGameSessionData(INetworkPlayer* p) { if (m_platform) m_platform->UpdateAndSetGameSessionData(p); }
void CGameNetworkManager::SendInviteGUI(int i) { if (m_platform) m_platform->SendInviteGUI(i); }
void CGameNetworkManager::ResetLeavingGame() { m_platform->ResetLeavingGame(); }
bool CGameNetworkManager::AlreadyInSession(const FriendPresence* p) { return m_platform && m_platform->AlreadyInSession(p); }
bool CGameNetworkManager::IsNetworkThreadRunning() { return m_networkThreadRunning; }
void* CGameNetworkManager::RunNetworkGameThreadProc(void* data)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    Block::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    FireworksRecipe::UseDefaultThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_NetworkGame);
    LevelChunk::CreateNewThreadStorage();

    sInstance.m_networkThreadRunning = true;
    const bool runResult = sInstance._RunNetworkGame(data);
    sInstance.m_networkThreadRunning = false;

    if (!runResult)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        TexturePack* selected = minecraft->mTexturePackRepository->getSelected();
        while (true)
        {
            if (!selected->isLoadingData())
            {
                if (!minecraft->mTexturePackRepository->needsUIUpdate() &&
                    !gConsoleUIController.IsReloadingSkin())
                {
                    gConsoleUIController.CleanUpSkinReload();
                    if (gApp.GetDisconnectReason() == 0)
                    {
                        gApp.SetDisconnectReason(
                            static_cast<DisconnectPacket::eDisconnectReason>(0x11));
                    }
                    if (gMinecraftServer == 0)
                    {
                        g_GameRuleManager.unloadCurrentGameRules();
                    }
                    Block::ReleaseThreadStorage();
                    return reinterpret_cast<void*>(static_cast<intptr_t>(-1));
                }
            }
            C4JThread::Sleep(1);
        }
    }

    while (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
        if (!mode->IsInTrasitionFromLobby())
        {
            break;
        }
        C4JThread::Sleep(5);
    }

    Block::ReleaseThreadStorage();
    IntCache::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    LevelChunk::ReleaseThreadStorage();
    return 0;
}
void* CGameNetworkManager::ExitAndJoinFromInviteThreadProc(void* data)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    UIScene_PauseMenu::_ExitWorld(NULL);
    while (sInstance.IsInSession())
        C4JThread::Sleep(1);

    Minecraft* minecraft = Minecraft::GetInstance();
    TexturePackRepository* repository = minecraft->mTexturePackRepository;
    TexturePack* previousSelected = repository->getSelected();
    previousSelected->unloadAudio();
    repository->selectTexturePackById(0, false);
    const bool loading = previousSelected->isLoadingData();
    SoundEngine* soundEngine = minecraft->mSoundEngine;
    if (!loading)
    {
        TexturePackRepository::defaultTexturePack->loadData();
        soundEngine->SetStreamingSounds(
            0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f, 0x20, 0x29, 0x2a,
            0x2a, 0x3e, 0x44, 0x57);
        soundEngine->playStreaming(NULL, 0.f, 0.f, 0.f, 1.f, 1.f, false, false,
                                   ConsoleSoundEngine::eAudioDelayType_None);
    }
    else
    {
        soundEngine->SetStreamingSounds(
            0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f, 0x20, 0x29, 0x2a,
            0x2a, 0x3e, 0x44, 0x57);
        soundEngine->playStreaming(NULL, 0.f, 0.f, 0.f, 1.f, 1.f, false, false,
                                   ConsoleSoundEngine::eAudioDelayType_None);
        TexturePackRepository::defaultTexturePack->loadData();
    }

    const int user = gApp.GetPrimaryUserIndex();
    while (gApp.GetXuiAction(user) != 0)
        C4JThread::Sleep(1);

    const int player = *static_cast<int*>(data);
    gApp.SetAction(player, static_cast<eXuiAction>(0x17), data);
    return NULL;
}
int CGameNetworkManager::TryToConnectOnline(int (*cb)(void*,bool,int), void* d, bool p) { if (m_platform) m_platform->TryToConnectOnline(cb,d,p); return 0; }
bool CGameNetworkManager::_LeaveGame() { return sInstance.LeaveGame(false); }
void* CGameNetworkManager::ChangeSessionTypeThreadProc(void*) { sInstance.LeaveGame(false); return 0; }
void CGameNetworkManager::SystemFlagSet(INetworkPlayer* p, int f) { if (m_platform) m_platform->SystemFlagSet(p,f); }
bool CGameNetworkManager::SystemFlagGet(INetworkPlayer* p, int f) { return m_platform && m_platform->SystemFlagGet(p,f); }
void CGameNetworkManager::SystemFlagClearAll(INetworkPlayer* p) { if (m_platform) m_platform->SystemFlagClearAll(p); }
void CGameNetworkManager::ServerReady() { if (m_serverReadyEvent) m_serverReadyEvent->Set(); }
void CGameNetworkManager::ServerStopped() { if (m_serverStoppedEvent) m_serverStoppedEvent->Set(); }
void CGameNetworkManager::ServerStoppedWait() { if (m_serverStoppedEvent) m_serverStoppedEvent->WaitForSignal(-1); }
void CGameNetworkManager::ServerStoppedDestroy() { delete m_serverStoppedEvent; m_serverStoppedEvent = 0; }
bool CGameNetworkManager::ServerStoppedValid() { return m_serverStoppedEvent != 0; }
void CGameNetworkManager::RemoveCreatedConnection(ClientPacketListener* p)
{
    EnterCriticalSection(&m_connectionsMutex);
    std::vector<ClientPacketListener*>::iterator i = m_connections.begin();
    while (i != m_connections.end() && *i != p)
    {
        ++i;
    }
    if (i != m_connections.end())
    {
        m_connections.erase(i);
    }
    LeaveCriticalSection(&m_connectionsMutex);
}
bool CGameNetworkManager::usingLanMode() { return m_platform && m_platform->usingLanMode(); }
bool CGameNetworkManager::setLanMode(bool v) { return m_platform && m_platform->setLanMode(v); }
bool CGameNetworkManager::isLanConnected() { return m_platform && m_platform->isLanConnected(); }
bool CGameNetworkManager::isLanModeChangeInProgress() { return m_platform && m_platform->isLanModeChangeInProgress(); }
std::wstring CGameNetworkManager::GatherStats() { return m_platform ? m_platform->GatherStats() : std::wstring(); }
void CGameNetworkManager::renderQueueMeter() {}
std::wstring CGameNetworkManager::GatherRTTStats() { return m_platform ? m_platform->GatherRTTStats() : std::wstring(); }
void CGameNetworkManager::SetSessionGameModeId(int id) { m_platform->SetSessionGameModeId(id); }
void CGameNetworkManager::StateChange_AnyToHosting() { gApp.SetRichPresenceContext(0, 0); }
void CGameNetworkManager::StateChange_AnyToJoining(bool) { gApp.SetRichPresenceContext(GetPrimaryPad(), 0); }
void CGameNetworkManager::StateChange_JoiningToIdle(CPlatformNetworkManager::eJoinFailedReason reason)
{
    AddLocalPlayerFailed(GetPrimaryPad(), reason == CPlatformNetworkManager::eJoinFailedReason_SessionFull);
}
void CGameNetworkManager::StateChange_AnyToStarting() { if (!IsHost()) RunNetworkGameThreadProc(0); }
void CGameNetworkManager::StateChange_AnyToEnding(bool error, bool)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft && minecraft->mGui)
    {
        minecraft->mGui->clearMessages(-1);
    }
    if (!IsHost() && !IsLeavingGame())
    {
        HandleDisconnect(error);
    }
}
Socket* CGameNetworkManager::CreateSocket(INetworkPlayer* player, bool local)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Socket* socket = player->GetSocket();
    if (socket && local)
    {
        socket->setPlayer(player);
        return socket;
    }

    socket = new Socket(player, IsHost(), IsHost() && local);
    player->SetSocket(socket);
    if (IsHost() && IsInGameplay())
    {
        Socket::addIncomingSocket(socket);
    }
    if (local && IsInGameplay())
    {
        int userIndex = player->GetUserIndex();
        ClientPacketListener* listener = new ClientPacketListener(minecraft, socket, userIndex);
        if (!listener->isStarted())
        {
            AddLocalPlayerFailed(userIndex, false);
            delete listener;
            return socket;
        }
        boost::shared_ptr<Packet> packet(new ServerboundPreLoginPacket(player->GetOnlineName()));
        listener->send(packet);
        minecraft->addPendingLocalConnection(userIndex, listener);
    }
    return socket;
}
void CGameNetworkManager::CloseConnection(INetworkPlayer* player)
{
    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
    {
        gMinecraftServer->getPlayers()->closeServerGamePacketListenerImplBySmallId(player->GetSmallId());
    }
}
void CGameNetworkManager::PlayerJoining(INetworkPlayer* player) { if (player->GetSocket() == 0) CreateSocket(player, player->IsLocal()); UpdateAndSetGameSessionData(player); }
void CGameNetworkManager::PlayerLeaving(INetworkPlayer* player) { if (player->IsLocal()) WriteStats(player); CloseConnection(player); }
void CGameNetworkManager::HostChanged() { gApp.SetAction(GetPrimaryPad(), (eXuiAction)3, (void*)1); }
void CGameNetworkManager::WriteStats(INetworkPlayer* player) { Minecraft::GetInstance()->forceStatsSave(player->GetUserIndex()); }
void CGameNetworkManager::AddLocalPlayerFailed(int userIndex, bool full)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mConnectionFailed[userIndex] = true;
    minecraft->mConnectionFailedMessage = full ? 0x0d : 0x11;
    minecraft->mConnectionFailedPlayerIdx = userIndex;
}
int CGameNetworkManager::GetPrimaryPad() { return gProfile->GetPrimaryPad(); }
int CGameNetworkManager::GetLockedProfile() { return gProfile->GetLockedProfile(); }
void CGameNetworkManager::HandleDisconnect(bool error)
{
    int primaryPad = GetPrimaryPad();
    if (GetLockedProfile() != -1 && primaryPad != -1 && IsInSession())
    {
        m_disconnectError = error;
        gApp.SetAction(primaryPad, (eXuiAction)0x10, 0);
    }
    else
    {
        m_disconnectError = false;
    }
}
bool CGameNetworkManager::IsSignedInLive(int userIndex) { return gProfile->IsSignedInLive(userIndex); }
bool CGameNetworkManager::AllowedToPlayMultiplayer(int userIndex) { return gProfile->AllowedToPlayMultiplayer(userIndex); }
void CGameNetworkManager::FakeLocalPlayerJoined() { if (m_platform) m_platform->FakeLocalPlayerJoined(); }
bool CGameNetworkManager::IsHandlingBootInvite() { return m_platform->IsHandlingBootInvite(); }
void CGameNetworkManager::WiiU_ClearIsHandlingBootInvite() { NQRNetworkManager_WiiU::ClearIsHandlingBootInvite(); }
void CGameNetworkManager::WiiU_ShowErrorViewer() { NQRNetworkManager_WiiU::ShowErrorViewer(); }
void CGameNetworkManager::ShowConnectOnlineNintendoProgress(int i, int (*cb)(void*,bool,int,int), void* d) { NQRNetworkManager_WiiU::ShowConnectOnlineNintendoProgress(i, cb, d); }
void CGameNetworkManager::WiiU_ReleaseForeground() { NintendoVoiceChat_WiiU::ReleaseForeground(); NQRNetworkManager_WiiU::SetAppSuspended(true); }
void CGameNetworkManager::WiiU_AcquiredForeground()
{
    NintendoVoiceChat_WiiU::AcquiredForeground();
    NQRNetworkManager_WiiU::SetAppSuspended(false);
    MasterGameMode::ResetLobbyTimerAfterSuspend();
}
void CGameNetworkManager::WiiU_ReleaseCallback() { NintendoVoiceChat_WiiU::ReleaseCallback(); }
void CGameNetworkManager::WiiU_SetBootFromFriendList() { gApp.setIgnoreMainMenuInputDuringInvite(true); }
bool CGameNetworkManager::IsUGCRestricted(int userIndex)
{
    bool explicitUser = userIndex != 0xff;
    if (!explicitUser)
    {
        userIndex = gProfile->GetPrimaryPad();
    }
    int allowed = 1;
    int friendsOnly = 1;
    gProfile->AllowedPlayerCreatedContent(userIndex, explicitUser, &allowed, &friendsOnly);
    return !allowed && !friendsOnly;
}
bool CGameNetworkManager::ModeAllowsUGC(EMiniGameId miniGameId)
{
    MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(miniGameId, true);
    return mode != 0 && mode->AllowsUGC();
}
bool CGameNetworkManager::IsUGCRestrictedMode(int i, EMiniGameId m) { return IsUGCRestricted(i) && ModeAllowsUGC(m); }
bool CGameNetworkManager::IsFriendsOnlyUGC(int userIndex)
{
    int allowed = 0;
    int friendsOnly = 0;
    gProfile->AllowedPlayerCreatedContent(userIndex, true, &allowed, &friendsOnly);
    return allowed != 1 && friendsOnly == 1;
}
void CGameNetworkManager::CheckFriendsCanPlay(int userIndex, int& friends,
                                               boost::shared_ptr<ClientboundPreLoginPacket> packet,
                                               int titleVersion, int& can, int& cannot,
                                               int& blocked, int& unknown)
{
    if (m_platform)
    {
        m_platform->CheckFriendsCanPlay(userIndex, friends, packet, titleVersion,
                                        can, cannot, blocked, unknown);
    }
}
unsigned int CGameNetworkManager::GetMaxPlayers(EMiniGameId miniGameId, unsigned int)
{
    if (miniGameId != eMiniGameId_Any)
    {
        MiniGameDef::GetCustomGameModeById(miniGameId, true);
    }
    return GameSessionData::MAX_SESSION_PLAYERS;
}
unsigned int CGameNetworkManager::GetMaxPlayers(GameSessionData* data)
{
    if (data->m_miniGameId != eMiniGameId_Any)
    {
        MiniGameDef::GetCustomGameModeById((EMiniGameId)data->m_miniGameId, true);
    }
    return GameSessionData::MAX_SESSION_PLAYERS;
}
unsigned int CGameNetworkManager::GetMaxPlayers()
{
    unsigned int players = GameSessionData::MAX_SESSION_PLAYERS;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        EMiniGameId id = (EMiniGameId)gApp.GetFirstMiniGameType();
        const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
        players = GetMaxPlayers(id, game->GetHostOption((eCustomGameModeHostOption)10));
    }
    return players;
}
void CGameNetworkManager::AttemptPSNSignIn(int (*cb)(void*,bool,int), void* d, bool p, int i) { if (m_platform) m_platform->AttemptPSNSignIn(cb,d,p,i); }
void CGameNetworkManager::RecvInviteGUI() { if (m_platform) m_platform->RecvInviteGUI(); }
void CGameNetworkManager::ClearInviteInfo() { m_inviteInfo = 0; }

namespace nn
{
namespace erreula
{
struct HomeNixSignArg
{
    int controller;
};

void AppearHomeNixSign(const HomeNixSignArg& arg);
}
}

extern bool CommerceAppletIsExecuting();
extern void CommerceAppletFinish();
extern int CommerceAppletGetReturnCode();

extern "C" unsigned int CallbackAcquiredForeground(void*)
{
    gRenderer->AcquiredForeground();
    g_pInput->AcquiredForeground();
    if (CommerceAppletIsExecuting())
    {
        CommerceAppletFinish();
        CommerceAppletGetReturnCode();
        cCommerceNintendo* commerce = static_cast<cCommerceNintendo*>(
            reinterpret_cast<void*>(gApp.GetCommerceInstance()));
        commerce->SetProcessState((cCommerceNintendo::eCommerce_Process_State)11);
    }
    CGameNetworkManager::sInstance.WiiU_AcquiredForeground();
    return 0;
}

unsigned int CallbackReleaseForeground__FPv(void*)
{
    gRenderer->ReleaseForeground();
    CGameNetworkManager::sInstance.WiiU_ReleaseForeground();
    return 0;
}

void closeSocketForLockedProfile__Fv()
{
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(
        gProfile->GetPrimaryPad());
    if (player != 0)
    {
        Socket* socket = player->GetSocket();
        if (socket != 0)
        {
            socket->close(true);
        }
    }
}

void closesocket__Fi(int socket)
{
    ::shutdown(socket, 2);
    ::close(socket);
}
