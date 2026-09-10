#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/File.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/StringFormat.hpp"
#include "game/world/biome/arrayWithLength.hpp"

extern CMinecraftApp* g_pApp;

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
    m_state78 = 0;
    m_state7c = 0;
}

void IUIScene_LoadCreateJoin::handleTimer(int timer)
{
    if (timer == 0)
        m_state19 = 0;
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
    m_state1a = 1;
}

void IUIScene_LoadCreateJoin::LoadGameMode(EMiniGameId gameMode)
{
    m_state34 = gameMode;
    m_state1a = 1;
}

void IUIScene_LoadCreateJoin::HandleLoadListPress(int index)
{
    m_state24 = index;
    m_state28 = 1;
}

void IUIScene_LoadCreateJoin::HandleNewListPress(int index, int variant)
{
    m_state24 = index;
    m_state34 = variant;
    m_state28 = 2;
}

void IUIScene_LoadCreateJoin::TexturePackDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
}

void IUIScene_LoadCreateJoin::CheckAndJoinGame(int index)
{
    m_state24 = index;
    m_state1a = 1;
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
