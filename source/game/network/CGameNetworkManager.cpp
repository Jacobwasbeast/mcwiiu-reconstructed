#include "game/network/CGameNetworkManager.hpp"
#include "game/core/StringIds.hpp"
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
#include "vendor/_ConnectionProgressParams.hpp"
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
#include "game/client/render/ProgressRenderer.hpp"
#include "game/util/Random.hpp"
#include "game/core/System.hpp"
#include "game/core/File.hpp"
#include "game/core/StringConversion.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "vendor/_NetworkGameInitData.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "platform/wiiu/CPlatformNetworkManagerNintendo.hpp"
#if defined(RECON_NATIVE)
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#endif

#include <sysapp/args.h>

extern "C" int shutdown(int socket, int how);
extern ConsoleUIController gConsoleUIController;
extern "C" int close(int socket);
void SetThreadName(unsigned int threadId, const char* name);
void closeSocketForLockedProfile();

CGameNetworkManager CGameNetworkManager::sInstance;

CGameNetworkManager::CGameNetworkManager()
    : m_initialised(false),
      m_pendingInviteUserIndex(0),
      m_pendingInviteUserMask(0),
      m_pendingInvitePresence(0),
      m_unknown18(-1),
      m_disconnectError(false),
      m_unknownState(false),
      m_publicJoinable(false),
      m_inviteInfo(0),
      m_connections(),
      m_joiningMiniGameId(eMiniGameId_Lobby)
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
int CGameNetworkManager::JoinFromInvite_SignInReturned(void* data, bool result, int userIndex)
{
    if (!result || !gProfile->IsSignedIn(userIndex) || !gProfile->IsSignedInLive(userIndex))
    {
        return 0;
    }
    unsigned int mask = 0;
    bool notAllowed = false;
    if (gProfile->IsSignedIn(0))
    {
        if (!gProfile->AllowedToPlayMultiplayer(0)) notAllowed = true;
        mask = GetLocalPlayerMask(0);
    }
    if (gProfile->IsSignedIn(1))
    {
        if (!gProfile->AllowedToPlayMultiplayer(1)) notAllowed = true;
        mask |= GetLocalPlayerMask(1);
    }
    if (gProfile->IsSignedIn(2))
    {
        if (!gProfile->AllowedToPlayMultiplayer(2)) notAllowed = true;
        mask |= GetLocalPlayerMask(2);
    }
    if (gProfile->IsSignedIn(3))
    {
        if (!gProfile->AllowedToPlayMultiplayer(3)) notAllowed = true;
        mask |= GetLocalPlayerMask(3);
    }
    if (notAllowed)
    {
        int messageParams = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &messageParams, 1,
                                                  gProfile->GetPrimaryPad(), 0, 0, 0, true);
        return 0;
    }
    gProfile->SetLockedProfile(userIndex);
    gProfile->SetPrimaryPad(userIndex);
    sInstance.SetLocalGame(false);
    gProfile->QuerySigninStatus();
    Minecraft::GetInstance()->clearConnectionFailed();
    sInstance.JoinGameFromInviteInfo(userIndex, mask, static_cast<const FriendPresence*>(data));
    return 0;
}
void CGameNetworkManager::HandleInviteWhenInMenus(int userIndex, const FriendPresence* presence)
{
    bool fullVersion = gProfile->IsFullVersion();
    gProfile->SetPrimaryPad(userIndex);
    if (!fullVersion)
    {
        gApp.SetAction(userIndex, eXuiAction_UpsellForInvite, 0);
        return;
    }
    gApp.StartInstallDLCProcess(userIndex);
    if (gApp.IsLocalMultiplayerAvailable() && !gApp.IsDrcPreventingSplitscreen())
    {
        return;
    }
    if (!gProfile->AllowedToPlayMultiplayer(userIndex))
    {
        int messageParams = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &messageParams, 1,
                                                  gProfile->GetPrimaryPad(), 0, 0, 0, true);
        return;
    }
    gProfile->SetLockedProfile(userIndex);
    gProfile->SetPrimaryPad(userIndex);
    unsigned int mask = GetLocalPlayerMask(userIndex);
    gProfile->QuerySigninStatus();
    Minecraft::GetInstance()->clearConnectionFailed();
    SetLocalGame(false);
    JoinGameFromInviteInfo(userIndex, mask, presence);
}
void CGameNetworkManager::ProcessInvite(unsigned int userIndex, unsigned int flags, FriendPresence* presence)
{
    m_pendingInvitePresence = presence;
    m_pendingInviteUserIndex = (int)userIndex;
    m_pendingInviteUserMask = flags;
    gApp.SetAction((int)userIndex, eXuiAction_JoinFromInvite, 0);
}
void CGameNetworkManager::GameInviteReceived(int userIndex, FriendPresence* presence)
{
    if (gConsoleUIController.IsShowingErrorMessage(userIndex))
    {
        if (presence != 0)
        {
            m_pendingInviteUserIndex = userIndex;
            m_pendingInvitePresence = presence;
        }
        return;
    }

    unsigned int mask = 0;
    m_pendingInvitePresence = presence;
    bool notAllowed = false;
    if (gProfile->IsSignedIn(0))
    {
        if (!gProfile->AllowedToPlayMultiplayer(0)) notAllowed = true;
        mask = GetLocalPlayerMask(0);
    }
    if (gProfile->IsSignedIn(1))
    {
        if (!gProfile->AllowedToPlayMultiplayer(1)) notAllowed = true;
        mask |= GetLocalPlayerMask(1);
    }
    if (gProfile->IsSignedIn(2))
    {
        if (!gProfile->AllowedToPlayMultiplayer(2)) notAllowed = true;
        mask |= GetLocalPlayerMask(2);
    }
    if (gProfile->IsSignedIn(3))
    {
        if (!gProfile->AllowedToPlayMultiplayer(3)) notAllowed = true;
        mask |= GetLocalPlayerMask(3);
    }
    if (notAllowed)
    {
        int messageParams = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &messageParams, 1, 0xff, 0, 0, 0, true);
        return;
    }
    if (!IsInSession())
    {
        HandleInviteWhenInMenus(userIndex, presence);
        return;
    }
    if (m_platform && m_platform->checkValidInviteData(presence))
    {
        ProcessInvite(userIndex, mask, presence);
    }
}
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
int CGameNetworkManager::ServerThreadProc(void* data)
{
    _NetworkGameInitData* initData = static_cast<_NetworkGameInitData*>(data);
    long long seed = 0;
    if (initData != 0)
    {
        seed = initData->m_seed;
        gApp.SetGameHostOption(eGameHostOption_All, initData->m_hostOptions);
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        miniGame->SetHostOption(eCustomGameModeHostOption_All, initData->m_miniGameHostOptions);
        if (initData->m_levelGenerationOptions != 0 &&
            initData->m_levelGenerationOptions->getParentPackId() != initData->m_texturePackId)
        {
            while (Minecraft::GetInstance()->mTexturePackRepository->needsUIUpdate() ||
                   gConsoleUIController.IsReloadingSkin())
                C4JThread::Sleep(1);
            initData->m_levelGenerationOptions->loadBaseSaveData();
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
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Server);
    LevelChunk::CreateNewThreadStorage();
    MinecraftServer::main(seed, initData);
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
bool CGameNetworkManager::StartNetworkGame(Minecraft* minecraft, void* data)
{
    _NetworkGameInitData* initData = static_cast<_NetworkGameInitData*>(data);
    if (initData != 0 && initData->m_levelGenerationOptions != 0)
    {
        gApp.setLevelGenerationOptions(initData->m_levelGenerationOptions);
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        if (options == 0)
        {
            closeSocketForLockedProfile();
            return false;
        }
        if (!options->hasLevelSeed())
        {
            Random random(System::processTimeInNanoSecs());
            initData->m_seed = random.nextLong();
#if defined(RECON_NATIVE)
            const char* fixture = std::getenv("MC_WORLD_SEED");
            if (fixture != NULL && *fixture != '\0')
            {
                char* end = NULL;
                errno = 0;
                const long long fixtureSeed = std::strtoll(fixture, &end, 10);
                if (errno == 0 && end != fixture && *end == '\0')
                    initData->m_seed = fixtureSeed;
                else
                    std::fprintf(stderr, "[native-world] invalid MC_WORLD_SEED: %s\n", fixture);
            }
#endif
        }
        else
        {
            initData->m_seed = options->getLevelSeed();
        }
        if (options->isLocal())
        {
            bool hasBaseSave = false;
            if (options->requiresBaseSave() && !options->getBaseSavePath().empty())
            {
                hasBaseSave = true;
            }
            if (hasBaseSave)
            {
                const unsigned int texturePackId = options->getTexturePackId();
                if (!options->isTutorial())
                {
                    if (texturePackId == 0)
                    {
                        std::wstring root = options->getBaseSaveRoot();
                        std::wstring path = options->getBaseSavePath();
                        std::wstring combined = root;
                        combined.append(path);
                        options->setBaseSaveData(File(combined));
                    }
                    else
                    {
                        std::wstring path = options->getBaseSavePath();
                        std::wstring root = options->getBaseSaveRoot();
                        std::wstring filePath =
                            gApp.getFilePath(texturePackId, path, true, root);
                        options->setBaseSaveData(File(filePath));
                    }
                }
                else
                {
                    std::wstring path = options->getBaseSavePath();
                    std::wstring root = options->getBaseSaveRoot();
                    std::wstring filePath = gApp.getFilePath(4, path, true, root);
                    options->setBaseSaveData(File(filePath));
                }
            }
        }
    }

    ServerStoppedCreate(false);
    if (!IsHost())
    {
        Socket::Initialise(0);
    }
    else
    {
        ServerStoppedCreate(true);
        ServerReadyCreate(true);
        C4JThreadImpl* serverThread =
            new C4JThreadImpl(ServerThreadProc, initData, "Server", 0x100000);
        serverThread->SetProcessor(0);
        serverThread->Run();
        serverThread->SetDeleteOnExit(true);
        ServerReadyWait();
        ServerReadyDestroy();
    }

    if (!IsReadyToPlayOrIdle())
    {
        ProgressRenderer* progressRenderer = minecraft->mProgressRenderer;
        do
        {
            progressRenderer->progressStage(CorrectErrorIDS(0xaebf0574));
            progressRenderer->progressStagePercentage(GetJoiningReadyPercentage());
            Sleep(10);
        } while (!IsReadyToPlayOrIdle());
        progressRenderer->progressStagePercentage(100);
    }

    if (!IsInSession())
    {
        MinecraftServer::HaltServer(false);
        closeSocketForLockedProfile();
        return false;
    }

    while (!gApp.DLCInstallProcessCompleted() && gApp.DLCInstallPending() && !IsLeavingGame())
    {
        Sleep(10);
    }
    if (IsLeavingGame())
    {
        MinecraftServer::HaltServer(false);
        closeSocketForLockedProfile();
        return false;
    }

    ClientPacketListener* primaryListener = 0;
    if (!IsHost())
    {
        INetworkPlayer* localPlayer = GetLocalPlayerByUserIndex(gProfile->GetLockedProfile());
        Socket* socket = localPlayer != 0 ? localPlayer->GetSocket() : 0;
        if (socket == 0)
        {
            MinecraftServer::HaltServer(false);
            return false;
        }
        primaryListener = new ClientPacketListener(minecraft, socket, -1);
    }
    else
    {
        primaryListener = new ClientPacketListener(minecraft, 0, -1);
    }
    if (!primaryListener->m_createdOk)
    {
        delete primaryListener;
        MinecraftServer::HaltServer(false);
        return false;
    }

    const int primaryPad = gProfile->GetPrimaryPad();
    std::wstring onlineName = convStringToWstring(gProfile->GetGamertag(primaryPad));
    boost::shared_ptr<Packet> preLoginPacket(new ServerboundPreLoginPacket(onlineName));
    primaryListener->send(preLoginPacket);

    TexturePack* selectedTexturePack = 0;
    if (!IsHost())
    {
        minecraft->mProgressRenderer->progressStage(IDS_PROGRESS_CONNECTING);
        selectedTexturePack = minecraft->mTexturePackRepository->getSelected();
    }
    else
    {
        gConsoleUIController.StartReloadSkinThread();
        selectedTexturePack = minecraft->mTexturePackRepository->getSelected();
    }

    while (true)
    {
        primaryListener->tick();
        CompressedBlockStorage::tick();
        SparseLightStorage::tick();
        SparseDataStorage::tick();
        Sleep(50);
        if (IsInSession() && !primaryListener->isStarted() && !primaryListener->isClosed() &&
            !IsLeavingGame())
        {
            continue;
        }
        if (selectedTexturePack->isLoadingData())
            continue;

        if (!minecraft->mTexturePackRepository->needsUIUpdate() &&
            !gConsoleUIController.IsReloadingSkin())
        {
            break;
        }
    }
    gConsoleUIController.CleanUpSkinReload();

    if (IsLeavingGame() || !IsInSession())
    {
        primaryListener->close();
    }

    if (!primaryListener->isStarted() || primaryListener->isClosed())
    {
        if (primaryListener->isClosed() || !IsInSession())
        {
            MinecraftServer::HaltServer(false);
            return false;
        }
        if (IsLeavingGame())
        {
            MinecraftServer::HaltServer(false);
            return false;
        }
    }
    else
    {
        EnterCriticalSection(&m_connectionsMutex);
        m_connections.push_back(primaryListener);
        LeaveCriticalSection(&m_connectionsMutex);

        const int hostPad = gProfile->GetPrimaryPad();
        const int playerCount = GetPlayerCount();
        const bool localGame = IsLocalGame();
        if (playerCount < 2)
            gProfile->SetCurrentGameActivity(hostPad, localGame ? 5 : 4, false);
        else
            gProfile->SetCurrentGameActivity(hostPad, localGame ? 3 : 2, false);

        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (userIndex == gProfile->GetPrimaryPad())
            {
                continue;
            }
            if (GetLocalPlayerByUserIndex(userIndex) != 0 && !gProfile->IsSignedIn(userIndex))
            {
                Socket* socket = GetLocalPlayerByUserIndex(userIndex)->GetSocket();
                if (socket != 0 && !socket->close(false))
                    socket->close(true);
                continue;
            }

            INetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
            Socket* socket = player != 0 ? player->GetSocket() : 0;
            if (player == 0 || socket == 0)
            {
                continue;
            }

            ClientPacketListener* listener = new ClientPacketListener(minecraft, socket, userIndex);
            if (!listener->m_createdOk)
            {
                continue;
            }

            minecraft->addPendingLocalConnection(userIndex, listener);
            Socket::addIncomingSocket(socket);
            std::wstring playerName = convStringToWstring(gProfile->GetGamertag(userIndex));
            boost::shared_ptr<Packet> playerPreLogin(new ServerboundPreLoginPacket(playerName));
            listener->send(playerPreLogin);

            EnterCriticalSection(&m_connectionsMutex);
            m_connections.push_back(listener);
            LeaveCriticalSection(&m_connectionsMutex);

            while (true)
            {
                EnterCriticalSection(&m_connectionsMutex);
                for (std::vector<ClientPacketListener*>::iterator it = m_connections.begin();
                     it != m_connections.end(); ++it)
                {
                    if (*it != 0)
                    {
                        (*it)->tick();
                    }
                }
                LeaveCriticalSection(&m_connectionsMutex);
                CompressedBlockStorage::tick();
                SparseLightStorage::tick();
                SparseDataStorage::tick();
                Sleep(50);
                if (!IsInSession() || listener->isStarted() || listener->isClosed() || IsLeavingGame())
                {
                    break;
                }
            }

            if (IsLeavingGame() || !IsInSession())
            {
                break;
            }

            if (!gProfile->IsSignedIn(userIndex) || listener->isClosed())
            {
                if (listener->isClosed())
                {
                    EnterCriticalSection(&m_connectionsMutex);
                    for (std::vector<ClientPacketListener*>::iterator it = m_connections.begin();
                         it != m_connections.end(); ++it)
                    {
                        if (*it == listener)
                        {
                            delete *it;
                            m_connections.erase(it);
                            break;
                        }
                    }
                    LeaveCriticalSection(&m_connectionsMutex);
                }
                else
                {
                    gApp.SetRichPresenceContext(userIndex, 0);
                    gProfile->SetCurrentGameActivity(userIndex, IsLocalGame() ? 3 : 2, false);
                }
            }
        }

        gApp.SetGameMode(static_cast<eGameMode>(1));
    }

    if (!IsLeavingGame() && IsInSession())
    {
        return true;
    }

    EnterCriticalSection(&m_connectionsMutex);
    for (std::vector<ClientPacketListener*>::iterator it = m_connections.begin();
         it != m_connections.end(); ++it)
    {
        if (*it != 0)
        {
            (*it)->close();
        }
    }
    LeaveCriticalSection(&m_connectionsMutex);
    MinecraftServer::HaltServer(false);
    return false;
}
bool CGameNetworkManager::_RunNetworkGame(void* data)
{
    const bool isHost = IsHost();
    Minecraft* minecraft = Minecraft::GetInstance();
    bool result = StartNetworkGame(minecraft, data);
    if (!result)
    {
        return false;
    }

    if (isHost)
    {
        result = m_platform->_RunNetworkGame();
        if (!result)
        {
            gApp.SetAction(GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
            return true;
        }
    }

    if (IsLeavingGame())
    {
        return false;
    }

    gApp.SetGameStarted(true);
    if (!gProfile->IsFullVersion())
    {
        gConsoleUIController.SetTrialTimerLimitSecs(0x960u);
        gApp.SetTrialTimerStart();
    }
    return result;
}
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
void CGameNetworkManager::HostGame(int a, bool b, bool c, unsigned char d, EMiniGameId e, unsigned int f, unsigned int* g)
{
    Minecraft::GetInstance()->clearConnectionFailed();
    if (m_platform)
        m_platform->HostGame(a, b, c, d, e, f, g);
}
bool CGameNetworkManager::IsInStatsEnabledSession() { return m_platform && m_platform->IsInStatsEnabledSession(); }
bool CGameNetworkManager::IsMatchmakingEnabled() { return m_platform && m_platform->IsMatchmakingEnabled(); }
bool CGameNetworkManager::IsInPublicJoinableGame() { return (m_publicJoinable || IsMatchmakingEnabled()) && !IsLocalGame(); }
void CGameNetworkManager::SetIsInPublicJoinableGame(bool value) { m_publicJoinable = value; }
bool CGameNetworkManager::IsInActiveCoopOrCompetitive(int)
{
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) &&
        (!mode->IsInActiveRoundState() || mode->IsRoundEnding() ||
         mode->IsRoundStarting() || mode->IsRoundRestarting()))
    {
        return false;
    }
    return true;
}
bool CGameNetworkManager::SessionHasSpace(unsigned int n) { return m_platform && m_platform->SessionHasSpace(n); }
bool CGameNetworkManager::SessionHasMii(int n) { return m_platform && m_platform->SessionHasMii(n); }
std::vector<FriendSessionInfo*>* CGameNetworkManager::GetSessionList(int a, int b, bool c) { return m_platform ? m_platform->GetSessionList(a,b,c) : 0; }
bool CGameNetworkManager::GetGameSessionInfo(int a, unsigned int b, FriendSessionInfo* c) { return m_platform && m_platform->GetGameSessionInfo(a,b,c); }
void CGameNetworkManager::SetSessionsUpdatedCallback(void (*cb)(void*), void* d) { if (m_platform) m_platform->SetSessionsUpdatedCallback(cb,d); }
void CGameNetworkManager::GetFullFriendSessionInfo(FriendSessionInfo* a, void (*cb)(bool,void*), void* d) { if (m_platform) m_platform->GetFullFriendSessionInfo(a,cb,d); }
void CGameNetworkManager::ForceFriendsSessionRefresh() { if (m_platform) m_platform->ForceFriendsSessionRefresh(); }
void CGameNetworkManager::ClearFriendsSessions() { if (m_platform) m_platform->ClearFriendsSessions(); }
int CGameNetworkManager::JoinGame(FriendSessionInfo* info, int userIndex, bool matchmaking)
{
    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
    SetLocalGame(false);
    int lockedProfile = gProfile->GetLockedProfile();
    Minecraft::GetInstance()->clearConnectionFailed();
    unsigned int localMask = GetLocalPlayerMask(gProfile->GetPrimaryPad());
    if (m_platform == 0)
    {
        return 0;
    }
    m_platform->JoinGame(info, userIndex | localMask, lockedProfile, matchmaking);
    return 0;
}
bool CGameNetworkManager::FindAndJoinGame(int userIndexMask, EMiniGameId miniGameId, unsigned int attributeCount, unsigned int* attributeValues, unsigned int* attributeIds)
{
    if (!gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
    {
        return true;
    }
    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
    SetLocalGame(false);
    int lockedProfile = gProfile->GetLockedProfile();
    Minecraft::GetInstance()->clearConnectionFailed();
    unsigned int localMask = GetLocalPlayerMask(gProfile->GetPrimaryPad());
    return m_platform && m_platform->FindAndJoinGame(userIndexMask | localMask, lockedProfile, miniGameId,
                                                       attributeCount, attributeValues, attributeIds);
}
void CGameNetworkManager::CancelJoinGame(void*)
{
    return;
}
bool CGameNetworkManager::LeaveGame(bool e) { Minecraft::GetInstance()->mGui->clearMessages(-1); return m_platform->LeaveGame(e); }
void CGameNetworkManager::UpdateAndSetGameSessionData(INetworkPlayer* p) { if (m_platform) m_platform->UpdateAndSetGameSessionData(p); }
void CGameNetworkManager::SendInviteGUI(int i) { if (m_platform) m_platform->SendInviteGUI(i); }
void CGameNetworkManager::ResetLeavingGame() { m_platform->ResetLeavingGame(); }
bool CGameNetworkManager::AlreadyInSession(const FriendPresence* p) { return m_platform && m_platform->AlreadyInSession(p); }
bool CGameNetworkManager::IsNetworkThreadRunning() { return m_networkThreadRunning; }
int CGameNetworkManager::RunNetworkGameThreadProc(void* data)
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
                            DisconnectPacket::eDisconnectReason_Banned);
                    }
                    if (gMinecraftServer == 0)
                    {
                        gApp.mGameRuleManager.unloadCurrentGameRules();
                    }
                    Block::ReleaseThreadStorage();
                    return -1;
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
int CGameNetworkManager::ExitAndJoinFromInviteThreadProc(void* data)
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

    const int user = gProfile->GetPrimaryPad();
    while (gApp.GetXuiAction(user) != eXuiAction_Idle)
        C4JThread::Sleep(1);

    const int player = *static_cast<int*>(data);
    gApp.SetAction(player, eXuiAction_JoinInvitedGame, data);
    return 0;
}
int CGameNetworkManager::TryToConnectOnline(int (*cb)(void*,bool,int), void* d, bool p) { if (m_platform) m_platform->TryToConnectOnline(cb,d,p); return 0; }
bool CGameNetworkManager::_LeaveGame() { return sInstance.LeaveGame(false); }
int CGameNetworkManager::ChangeSessionTypeThreadProc(void*)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();

    int primaryPad = gProfile->GetPrimaryPad();
    while (gApp.GetXuiServerAction(primaryPad) != 0)
    {
        C4JThread::Sleep(10);
        primaryPad = gProfile->GetPrimaryPad();
    }
    gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer,
                             reinterpret_cast<void*>(1));

    while (sInstance.IsInSession())
    {
        C4JThread::Sleep(1);
    }

    sInstance.SetLocalGame(true);
    unsigned int localMask = 0;
    int localCount = 0;
    for (int userIndex = 0; userIndex < 4; ++userIndex)
    {
        if (gProfile->IsSignedIn(userIndex))
        {
            ++localCount;
            localMask |= CGameNetworkManager::GetLocalPlayerMask(userIndex);
        }
    }

    while (sInstance.GetPlayerCount() < localCount)
    {
        C4JThread::Sleep(1);
    }

    while (gConsoleUIController.IsSceneInStack(0xff, EUIScene_MessageBox))
    {
        C4JThread::Sleep(10);
    }

    gApp.SetGameStarted(true);
    gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer, 0);
    gApp.SetChangingSessionType(false);
    gApp.SetReallyChangingSessionType(false);
    return 0;
}
void CGameNetworkManager::SystemFlagSet(INetworkPlayer* p, int f) { if (m_platform) m_platform->SystemFlagSet(p,f); }
bool CGameNetworkManager::SystemFlagGet(INetworkPlayer* p, int f) { return m_platform && m_platform->SystemFlagGet(p,f); }
void CGameNetworkManager::SystemFlagClearAll(INetworkPlayer* p) { if (m_platform) m_platform->SystemFlagClearAll(p); }
void CGameNetworkManager::ServerReady() { if (m_serverReadyEvent) m_serverReadyEvent->Set(); }
void CGameNetworkManager::ServerStopped() { if (m_serverStoppedEvent) m_serverStoppedEvent->Set(); }
void CGameNetworkManager::ServerStoppedWait()
{
    if (!m_serverStoppedEvent)
    {
        return;
    }
    if (!C4JThread::isMainThread())
    {
        m_serverStoppedEvent->WaitForSignal(-1);
        return;
    }
    unsigned int result;
    do
    {
        result = m_serverStoppedEvent->WaitForSignal(0x14);
        gConsoleUIController.tick();
        gConsoleUIController.render();
    } while (result == 0x102);
}
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
void CGameNetworkManager::renderQueueMeter()
{
    return;
}
std::wstring CGameNetworkManager::GatherRTTStats() { return m_platform ? m_platform->GatherRTTStats() : std::wstring(); }
void CGameNetworkManager::SetSessionGameModeId(int id) { m_platform->SetSessionGameModeId(id); }
void CGameNetworkManager::StateChange_AnyToHosting() { gApp.SetRichPresenceContext(0, 0); }
void CGameNetworkManager::StateChange_AnyToJoining(bool)
{
    XEnableGuestSignin(0);
    Minecraft::GetInstance()->mPendingClientTextureRequests.clear();

    _ConnectionProgressParams* params = new _ConnectionProgressParams();
    params->m_stringId = -1;
    params->m_playerId = gProfile->GetPrimaryPad();
    params->m_showTooltips = false;
    params->m_setFailTimer = true;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame))
        params->m_miniGameId = gApp.GetFirstMiniGameType();
    params->m_timerTime = 500;
    gConsoleUIController.CloseUIScenes(gProfile->GetPrimaryPad(), false);
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_ConnectingProgress, params,
                                         EUILayer_Scene, EUIGroup_PlayerDefault);
}
void CGameNetworkManager::StateChange_JoiningToIdle(CPlatformNetworkManager::eJoinFailedReason reason)
{
    int primaryPad = gProfile->GetPrimaryPad();
    if (reason == CPlatformNetworkManager::eJoinFailedReason_NotAllowedOnline)
    {
        gApp.SetAction(primaryPad, eXuiAction_MultiplayerNotAllowed, 0);
    }
    AddLocalPlayerFailed(primaryPad, reason == CPlatformNetworkManager::eJoinFailedReason_SessionFull);
}
void CGameNetworkManager::StateChange_AnyToStarting()
{
    if (IsHost())
    {
        return;
    }
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_threadParam = 0;
    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->showTip = true;
    progressState->closeMode = 3;
    progressState->playerId = gProfile->GetPrimaryPad();
    progressState->backScene = EUIScene_None;
    params->m_completionData = progressState;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame))
    {
        params->m_miniGameId = gApp.GetFirstMiniGameType();
    }
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                          EUILayer_Scene, EUIGroup_PlayerDefault);
}
void CGameNetworkManager::StateChange_AnyToEnding(bool, bool endSession)
{
    if (endSession && IsLocalGame())
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            INetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
            if (player != 0 && gProfile->IsSignedIn(userIndex))
            {
                Minecraft::GetInstance()->forceStatsSave(player->GetUserIndex());
            }
        }
    }
    Minecraft::GetInstance()->mGui->clearMessages(-1);
    if (!IsHost() && !IsLeavingGame())
    {
        if (gApp.GetDisconnectReason() == 0)
        {
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_Quit);
        }
        gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
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
void CGameNetworkManager::PlayerJoining(INetworkPlayer* player)
{
    if (IsInGameplay())
    {
        bool few = GetPlayerCount() < 2;
        bool localGame = IsLocalGame();
        int activityCode = few ? (localGame ? 5 : 4) : (localGame ? 3 : 2);
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (GetLocalPlayerByUserIndex(userIndex) != 0)
            {
                gApp.SetRichPresenceContext(userIndex, 0);
                gProfile->SetCurrentGameActivity(userIndex, activityCode, false);
            }
        }
    }
}
void CGameNetworkManager::PlayerLeaving(INetworkPlayer* player)
{
    if (player->IsLocal())
    {
        gProfile->SetCurrentGameActivity(player->GetUserIndex(), 0, false);
    }
}
void CGameNetworkManager::HostChanged() { gApp.SetAction(GetPrimaryPad(), eXuiAction_ExitWorld, (void*)1); }
void CGameNetworkManager::WriteStats(INetworkPlayer* player) { Minecraft::GetInstance()->forceStatsSave(player->GetUserIndex()); }
void CGameNetworkManager::AddLocalPlayerFailed(int userIndex, bool full)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mConnectionFailed[userIndex] = true;
    minecraft->mDisconnectReasons[userIndex] =
        full ? DisconnectPacket::eDisconnectReason_GameFull : DisconnectPacket::eDisconnectReason_Banned;
}
int CGameNetworkManager::GetPrimaryPad() { return gProfile->GetPrimaryPad(); }
int CGameNetworkManager::GetLockedProfile() { return gProfile->GetLockedProfile(); }
void CGameNetworkManager::HandleDisconnect(bool error)
{
    int primaryPad = GetPrimaryPad();
    if (GetLockedProfile() != -1 && primaryPad != -1 && IsInSession())
    {
        m_disconnectError = error;
        gApp.SetAction(primaryPad, eXuiAction_ConnectionLost, 0);
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
void CGameNetworkManager::WiiU_SetBootFromFriendList()
{
    SYSArgDataBlock args[2] = {};
    args[0].id = SYS_ARG_ID_JOIN_PID;
    args[1].id = SYS_ARG_ID_END;

    SYSCallerInfo callerInfo = {};
    if (SYSGetArguments(args, &callerInfo) == 0 && args[0].type == SYS_ARG_TYPE_UINT32)
    {
        g_bSafeToRespondToGameBootInvite = true;
        g_iGameBootInviteUserIndex = static_cast<int>(args[0].uint32);
        gApp.setIgnoreMainMenuInputDuringInvite(true);
    }
}
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
        players = GetMaxPlayers(id, game->GetHostOption(eCustomGameModeHostOption_All));
    }
    return players;
}
void CGameNetworkManager::AttemptPSNSignIn(int (*cb)(void*,bool,int), void* d, bool p, int i) { if (m_platform) m_platform->AttemptPSNSignIn(cb,d,p,i); }
void CGameNetworkManager::RecvInviteGUI() { if (m_platform) m_platform->RecvInviteGUI(); }
void CGameNetworkManager::ClearInviteInfo()
{
    m_joiningMiniGameId = eMiniGameId_Lobby;
    m_inviteInfo = 0;
}

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

unsigned int CallbackAcquiredForeground(void*)
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

unsigned int CallbackReleaseForeground(void*)
{
    gRenderer->ReleaseForeground();
    CGameNetworkManager::sInstance.WiiU_ReleaseForeground();
    return 0;
}

void closeSocketForLockedProfile()
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

void closesocket(int socket)
{
    ::shutdown(socket, 2);
    ::close(socket);
}
