#include "game/core/IUIScene_LoadCreateJoin.hpp"

#include <new>
#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringFormat.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LevelGenerators.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_NetworkGameInitData.hpp"

extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;
extern CProfile* gProfile;
extern CStorage* gStorage;

namespace
{
struct FullscreenProgressState
{
    bool showConfirmation;
    bool unknownState1;
    bool unknownState2;
    bool showTip;
    int closeMode;
    int playerId;
    EUIScene backScene;
};

struct LoadMenuSceneData
{
    int playerId;
    int saveIndex;
    void* saveData;
    int reserved;
    EMiniGameId gameMode;
};

struct JoinSceneData
{
    FriendSessionInfo* session;
    int userIndex;
    unsigned char customGame;
};

struct DiskSaveBuffer
{
    unsigned char* data;
    unsigned int length;
    std::wstring name;
};

std::vector<File*>& loadFileList(IUIScene_LoadCreateJoin* scene)
{
    return *reinterpret_cast<std::vector<File*>*>(
        reinterpret_cast<char*>(scene) + 92);
}

char*& saveDataBase(IUIScene_LoadCreateJoin* scene)
{
    return *reinterpret_cast<char**>(reinterpret_cast<char*>(scene) + 12);
}

int& selectedCustomGameModeId(IUIScene_LoadCreateJoin* scene)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(scene) + 120);
}

UIScene_LoadCreateJoinMenu* asMenu(IUIScene_LoadCreateJoin* scene)
{
    return static_cast<UIScene_LoadCreateJoinMenu*>(scene);
}

int texturePackIdAtSession(FriendSessionInfo* session)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(session) + 0xf4);
}
}

IUIScene_LoadCreateJoin::IUIScene_LoadCreateJoin()
    : m_state0(0), m_state4(0), m_userIndex(0), m_state10(0), m_state14(0), m_ignoreInput(0),
      m_state19(0), m_state1a(0), m_state1b(0), m_state1c(0), m_state1d(0), m_state1e(0),
      m_state1f(0), m_customGame(0), m_state21(0), m_state22(0), m_state23(0), m_state24(0),
      m_state28(0), m_state2c(0), m_state30(0), m_state34(0), m_state38(0), m_state3c(0),
      m_state40(0), m_state44(0), m_state48(0), m_state70(1), m_state71(0), m_state72(0),
      m_state73(0), m_state74(0), m_state78(0), m_state79(0), m_state7a(0), m_state7b(0),
      m_state7c(0), m_state7d(0), m_state7e(0), m_state7f(0), m_state80(0), m_state81(0),
      m_state82(0), m_state83(0), m_state84(0), m_state85(0), m_state86(0), m_state87(0),
      m_state88(0), m_state89(0), m_state8a(0), m_state8b(0), m_state8c(0), m_state9c(0),
      m_statea0(0), m_statea1(0), m_statea2(0), m_statea3(0), m_statea4(0), m_statea8(0)
{
}

IUIScene_LoadCreateJoin::~IUIScene_LoadCreateJoin()
{
    m_friendSessions.clear();
    m_levelGenerationOptions.clear();
    m_buttonIds.clear();
}

uint32_t IUIScene_LoadCreateJoin::GetCustomGameModeID()
{
    return m_customGame ? 1U : 15U;
}

void IUIScene_LoadCreateJoin::UpdateGamesList()
{
    m_state1b = 1;
    m_friendSessions.clear();
    m_state1b = 0;
}

void IUIScene_LoadCreateJoin::UpdateGamesListCallback(void* context)
{
    if (context != 0)
        static_cast<IUIScene_LoadCreateJoin*>(context)->UpdateGamesList();
}

void IUIScene_LoadCreateJoin::_init(bool customGame)
{
    m_customGame = customGame;
    m_ignoreInput = 0;
    m_state19 = 0;
    m_state1a = 0;
    m_state1b = 0;
    m_state1c = 0;
    m_state24 = 0;
    m_state28 = 0;
    m_state2c = 0;
    m_state48 = 0;
    UpdateGamesList();
}

uint64_t IUIScene_LoadCreateJoin::AddMatchmakingButton(std::wstring label, int id, std::wstring description)
{
    m_buttonIds.push_back(id);
    return m_buttonIds.size();
}

uint64_t IUIScene_LoadCreateJoin::AddGameModeButton(std::wstring label, int id, std::wstring description)
{
    AddCreateListButton(label, id);
    std::wstring path = formatwstr(L"Graphics\\%ls.png", description.c_str());
    if (g_pApp->hasArchiveFile(path, false))
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
        if (file.data != 0 && file.length != 0)
            SetCreateListIcon(m_state2c, file.data, file.length, description);
    }
    ++m_state2c;
    return 0;
}

void IUIScene_LoadCreateJoin::AddDefaultButtons()
{
    m_buttonIds.clear();
    AddGameModeButton(std::wstring(), 0, std::wstring());
    AddMatchmakingButton(std::wstring(), 1, std::wstring());
}

uint32_t IUIScene_LoadCreateJoin::HandleTelemetrySavesReturned(void* context, SAVE_DETAILS* details, bool complete)
{
    (void)complete;
    void* manager = *reinterpret_cast<void**>(reinterpret_cast<char*>(g_pApp) + 0x24);
    void* callback = *reinterpret_cast<void**>(reinterpret_cast<char*>(manager) + 0x114);
    typedef void (*SaveCallback)(void*, void*, SAVE_DETAILS*, int);
    reinterpret_cast<SaveCallback>(callback)(manager, context, details, 0);
    return 0;
}

uint64_t IUIScene_LoadCreateJoin::GetSaveInfo()
{
    m_state1c = 0;
    return 0;
}

void IUIScene_LoadCreateJoin::_tick()
{
    if (m_state28 == 1)
    {
        m_state30 = 0;
        m_state2c = 0;
        m_ignoreInput = 0;
        m_state1a = 0;
        m_state28 = 0;
        GetSaveInfo();
    }
    else if (m_state28 == 2)
    {
        m_state1b = 0;
        m_state2c = 0;
        m_state1a = 0;
        m_state30 = 0;
        m_ignoreInput = 0;
        m_state24 = 0;
        GetSaveInfo();
        m_state28 = 0;
    }
    else if (m_state28 != 0 && m_state28 <= 4)
    {
        m_state30 = 0;
        m_state1a = 0;
        m_state8c = 0;
        m_state24 = 0;
        m_state2c = 0;
        m_state1b = 0;
        GetSaveInfo();
        m_state28 = 0;
    }
}

void IUIScene_LoadCreateJoin::_updateTooltips()
{
    UIScene_LoadCreateJoinMenu* menu = asMenu(this);
    int tooltip3 = -1;
    int tooltip4 = -1;
    if (menu->getSubSceneType() == 0 || menu->GetPad() == 0)
    {
        if (m_customGame == 0 && menu->DoesLoadListHaveFocus() && menu->DoesCreateListHaveFocus())
        {
            if (gStorage->GetSaveDisabled() == 0 && gStorage->EnoughSpaceForAMinSaveGame() != 0)
                tooltip4 = 0x0aa027cc;
            else
                tooltip4 = 0x0e3265e5;
        }
        else if (menu->GetHideLoadTab())
            tooltip4 = 0x0f131e015;
    }
    else
        tooltip4 = 0x0aec93217;

    if (m_state1e != 0 && menu->getSubSceneType() != 0)
        tooltip3 = m_state1f != 0 ? 0x060f9906e : 0x051758df1;

    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()) && gStorage->GetSaveDisabled() != 0)
        menu->DoesLoadListHaveFocus();

    gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, tooltip3, tooltip4, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void IUIScene_LoadCreateJoin::handleTimer(int timer)
{
    if (timer != 0)
        return;

    UIScene_LoadCreateJoinMenu* menu = asMenu(this);
    unsigned char signedLive = 0;
    int pad = menu->GetPad();
    if (gProfile->IsSignedIn(pad) && gProfile->IsSignedInLive(pad))
        signedLive = 1;
    if (signedLive == m_state19)
        return;
    if (signedLive == 0)
    {
        m_state1e = 0;
        menu->handleReload();
        menu->ShowSaveListTimer(false);
        menu->ShowJoinListTimer(true);
        menu->ShowNoGamesLabel(false);
    }
    m_state19 = signedLive;
}

uint32_t IUIScene_LoadCreateJoin::DeleteSaveDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    if (context != 0)
        static_cast<IUIScene_LoadCreateJoin*>(context)->m_state1e = 0;
    return 0;
}

void IUIScene_LoadCreateJoin::SaveOptionsDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
}

uint32_t IUIScene_LoadCreateJoin::handleKeyDown(int key, int action, bool repeat)
{
    if (m_ignoreInput != 0 || (m_state1b == 0 && m_customGame == 0))
        return 0;
    if (action == 1 || action == 21)
        return 1;
    if (action == 3 || action == 6 || action == 10)
    {
        ++m_state3c;
        if (m_state3c >= 3)
            m_state3c = 0;
    }
    return 0;
}

void IUIScene_LoadCreateJoin::LoadLevelGen(LevelGenerationOptions* options)
{
    m_levelGenerationOptions.push_back(options);
    m_state1a = 1;
}

void IUIScene_LoadCreateJoin::LoadSaveFromDisk(File* file, ESavePlatform platform)
{
    gStorage->Tick();
    std::wstring saveName = file->getName();
    gStorage->SetSaveTitle(saveName.c_str());

    FileInputStream input(*file);
    unsigned int fileLength = static_cast<unsigned int>(file->length());
    arrayWithLength<unsigned char> fileData;
    fileData.data = new unsigned char[fileLength];
    fileData.length = fileLength;
    input.read(fileData);
    input.close();

    boost::shared_ptr<Player> player;
    bool hostOnline = false;
    if (gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
        hostOnline = true;
    g_pApp->SetTutorialMode(false, player);
    if (g_pApp->GetTutorialMode(player) != 0)
        hostOnline = false;

    g_pApp->SetGameHostOption(static_cast<eGameHostOption>(6), GameType::SURVIVAL->getId());
    unsigned char gameType =
        static_cast<unsigned char>(g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6)));
    CGameNetworkManager::sInstance.HostGame(0, hostOnline, false, gameType,
                                            static_cast<EMiniGameId>(15), 0, 0);

    DiskSaveBuffer* saveBuffer = new DiskSaveBuffer();
    saveBuffer->data = fileData.data;
    saveBuffer->length = fileLength;
    saveBuffer->name = saveName;

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_value2 = reinterpret_cast<int>(saveBuffer);
    initData->m_value3 = g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x20));
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    initData->m_value4 =
        miniGame->GetHostOption(static_cast<eCustomGameModeHostOption>(10));
    initData->m_value8 = static_cast<int>(platform);
    initData->m_value9 = g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6));

    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();

    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_firstParameter = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_secondParameter = initData;

    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->unknownState1 = true;
    progressState->unknownState2 = true;
    progressState->showTip = true;
    progressState->closeMode = 3;
    progressState->playerId = 0;
    progressState->backScene = static_cast<EUIScene>(0x53);
    params->m_thirdParameter = progressState;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(4),
                                           params, static_cast<EUILayer>(6),
                                           static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::LoadGameMode(EMiniGameId gameMode)
{
    LevelGenerationOptions* lobbyOptions = 0;
    std::vector<LevelGenerationOptions*>* generators = g_pApp->getLevelGenerators()->getLevelGenerators();
    if (generators != 0)
    {
        for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin();
             it != generators->end(); ++it)
        {
            if ((*it) != 0 && (*it)->isGameModeLobby())
                lobbyOptions = *it;
        }
    }

    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->playerId = asMenu(this)->GetPad();
    sceneData->saveIndex = -1;
    sceneData->saveData = lobbyOptions;
    sceneData->reserved = 0;
    sceneData->gameMode = gameMode;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x23),
                                       sceneData, static_cast<EUILayer>(6),
                                       static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::HandleLoadListPress(int index)
{
    m_state1a = 1;
    if (m_customGame != 0)
    {
        int* gameModeIds = reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 100);
        int gameMode = gameModeIds[index];
        if (gameMode > 4)
            gameMode = 4;
        selectedCustomGameModeId(this) = gameMode;
        LoadGameMode(static_cast<EMiniGameId>(gameMode));
        return;
    }

    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    if (g_pApp->DebugSettingsOn() != 0 && g_pApp->GetLoadSavesFromFolderEnabled() != 0)
    {
        boost::shared_ptr<Player> player;
        g_pApp->SetTutorialMode(false, player);
        File* loadFile = loadFileList(this).at(index);
        LoadSaveFromDisk(loadFile, static_cast<ESavePlatform>(0x55494957));
        return;
    }

    boost::shared_ptr<Player> player;
    g_pApp->SetTutorialMode(false, player);

    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->playerId = asMenu(this)->GetPad();
    sceneData->saveIndex = index;
    sceneData->saveData = saveDataBase(this) + index * 300;
    sceneData->reserved = 0;
    sceneData->gameMode = static_cast<EMiniGameId>(15);

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x23),
                                       sceneData, static_cast<EUILayer>(6),
                                       static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::HandleNewListPress(int index, int variant)
{
    m_state24 = index;
    m_state34 = variant;
    m_state28 = 2;
}

int IUIScene_LoadCreateJoin::TexturePackDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    return 0;
}

void IUIScene_LoadCreateJoin::CheckAndJoinGame(int index)
{
    UIScene_LoadCreateJoinMenu* menu = asMenu(this);
    if (menu->GetPad() == 0)
        return;
    if (m_friendSessions.empty())
        return;
    if (index < 0 || static_cast<unsigned int>(index) >= m_friendSessions.size())
        return;

    JoinSceneData* joinData = reinterpret_cast<JoinSceneData*>(static_cast<uintptr_t>(m_userIndex));
    if (joinData == 0)
        return;

    joinData->userIndex = menu->GetPad();
    FriendSessionInfo* session = m_friendSessions.at(index);
    joinData->customGame = m_customGame;
    joinData->session = session;

    if (texturePackIdAtSession(session) != 0)
    {
        TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
        unsigned int packCount = repository->getTexturePackCount();
        bool texturePackFound = false;
        for (unsigned int packIndex = 0; packIndex < packCount; ++packIndex)
        {
            TexturePack* pack = repository->getTexturePackByIndex(packIndex);
            if (pack != 0 && pack->getId() == texturePackIdAtSession(session))
            {
                texturePackFound = true;
                break;
            }
        }
        if (!texturePackFound)
        {
            int parameters[2];
            parameters[0] = 0xeac05ed4;
            parameters[1] = 0x4fcbc7e7;
            gConsoleUIController.RequestAlertMessage(
                0x1e8ee39f, 0x5ab46118, parameters, 2, menu->GetPad(),
                TexturePackDialogReturned, this, 0, true);
            return;
        }
    }
    menu->InitFocusJoinList(0);
    m_ignoreInput = 1;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x1c),
                                       joinData, static_cast<EUILayer>(6),
                                       static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::HandleGamesListPressed(int index)
{
    m_ignoreInput = 1;
    m_state2c = 2;
    CheckAndJoinGame(index);
}

void IUIScene_LoadCreateJoin::SetIgnoreInput(bool ignore)
{
    m_ignoreInput = ignore;
}
