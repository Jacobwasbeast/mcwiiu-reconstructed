#include "game/core/IUIScene_LoadCreateJoin.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/File.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/TexturePackRepository.hpp"

IUIScene_LoadCreateJoin::IUIScene_LoadCreateJoin()
    : m_primaryVtable(0), m_secondaryVtable(0), m_userIndex(0), m_selectedGame(0),
      m_selectedSave(0), m_ignoreInput(0), m_state19(0), m_state1a(0), m_state1b(0),
      m_state1c(0), m_state1d(0), m_state1e(0), m_state1f(0), m_customGame(0), m_state21(0),
      m_state22(0), m_state23(0), m_activeList(0), m_timer(0),
      m_gameMode(0), m_selectedTexturePack(0), m_currentButton(0), m_tooltipState(0),
      m_pendingAction(0), m_status(0), m_state58(0), m_state5c(0), m_ready(1), m_state71(0),
      m_state72(0), m_state73(0), m_state6c(0), m_state74(0), m_state78(0), m_state79(0), m_state7a(0),
      m_state7b(0), m_state7c(0), m_state7d(0), m_state7e(0), m_state7f(0), m_state80(0),
      m_state81(0), m_state82(0), m_state83(0), m_state84(0), m_state85(0), m_state86(0),
      m_state87(0), m_state88(0), m_state89(0), m_state8a(0), m_state8b(0), m_saveData(0),
      m_state90(0), m_state91(0), m_state92(0), m_state93(0), m_state94(0), m_state95(0),
      m_state96(0), m_state97(0), m_state98(0), m_state99(0), m_state9a(0), m_state9b(0),
      m_texturePack(0), m_statea0(0), m_statea1(0), m_statea2(0), m_statea3(0), m_scene(0),
      m_statea8(0), m_statea9(0), m_stateaa(0), m_stateab(0)
{
}

IUIScene_LoadCreateJoin::~IUIScene_LoadCreateJoin()
{
    m_friendSessions.clear();
    m_levelGenerationOptions.clear();
    m_buttonIds.clear();
    m_scene = 0;
    m_texturePack = 0;
}

uint32_t IUIScene_LoadCreateJoin::GetCustomGameModeID()
{
    return m_customGame ? 1U : 15U;
}

void IUIScene_LoadCreateJoin::UpdateGamesList()
{
    m_state1b = 1;
    m_friendSessions.clear();
    m_activeList = 0;
    m_state1b = 0;
}

void IUIScene_LoadCreateJoin::UpdateGamesListCallback(void* context)
{
    IUIScene_LoadCreateJoin* scene = static_cast<IUIScene_LoadCreateJoin*>(context);
    if (scene != 0)
        scene->UpdateGamesList();
}

void IUIScene_LoadCreateJoin::_init(bool customGame)
{
    m_customGame = customGame;
    m_ignoreInput = false;
    m_state1a = 0;
    m_pendingAction = 0;
    m_selectedGame = 0;
    m_selectedSave = 0;
    m_activeList = 0;
    m_status = 0;
    UpdateGamesList();
}

uint64_t IUIScene_LoadCreateJoin::AddMatchmakingButton(std::wstring label, int id,
                                                        std::wstring description)
{
    if (!label.empty() && !description.empty())
        m_buttonIds.push_back(id);
    else
        m_buttonIds.push_back(id);
    return static_cast<uint64_t>(m_buttonIds.size());
}

uint64_t IUIScene_LoadCreateJoin::AddGameModeButton(std::wstring label, int id,
                                                     std::wstring description)
{
    if (label.size() + description.size() >= 0)
        m_buttonIds.push_back(id);
    return static_cast<uint64_t>(m_buttonIds.size());
}

void IUIScene_LoadCreateJoin::AddDefaultButtons()
{
    m_buttonIds.clear();
    AddGameModeButton(std::wstring(), 0, std::wstring());
    AddMatchmakingButton(std::wstring(), 1, std::wstring());
    m_currentButton = 0;
}

uint32_t IUIScene_LoadCreateJoin::HandleTelemetrySavesReturned(void* context,
                                                                SAVE_DETAILS* details,
                                                                bool complete)
{
    if (context != 0 && details != 0 && complete)
        return 0;
    return 0;
}

uint64_t IUIScene_LoadCreateJoin::GetSaveInfo()
{
    m_state1c = m_buttonIds.empty() ? 0 : 1;
    return static_cast<uint64_t>(m_state1c);
}

void IUIScene_LoadCreateJoin::_tick()
{
    if (m_pendingAction == 1)
    {
        m_selectedSave = 0;
        m_selectedGame = 0;
        m_ignoreInput = false;
        m_pendingAction = 0;
        GetSaveInfo();
    }
    else if (m_pendingAction == 2)
    {
        m_selectedSave = 0;
        m_activeList = 0;
        m_ignoreInput = false;
        GetSaveInfo();
        m_pendingAction = 0;
    }
}

void IUIScene_LoadCreateJoin::_updateTooltips()
{
    m_tooltipState = m_ignoreInput ? -1 : (m_state1c ? 1 : 0);
}

void IUIScene_LoadCreateJoin::handleTimer(int timer)
{
    if (timer == 0)
    {
        m_state19 = m_status != 0;
        if (!m_state19)
            m_ready = 0;
    }
}

uint32_t IUIScene_LoadCreateJoin::DeleteSaveDialogReturned(void* context, int userIndex,
                                                            C4JStorage::EMessageResult result)
{
    IUIScene_LoadCreateJoin* scene = static_cast<IUIScene_LoadCreateJoin*>(context);
    if (scene != 0 && result == C4JStorage::EMessageResult_5)
    {
        scene->m_state1e = 0;
        scene->m_selectedSave = userIndex;
    }
    return 0;
}

void IUIScene_LoadCreateJoin::SaveOptionsDialogReturned(void* context, int userIndex,
                                                         C4JStorage::EMessageResult result)
{
    IUIScene_LoadCreateJoin* scene = static_cast<IUIScene_LoadCreateJoin*>(context);
    if (scene != 0)
        scene->m_state1f = result == C4JStorage::EMessageResult_5 ? 0 : 1;
    scene = scene;
    userIndex = userIndex;
}

uint32_t IUIScene_LoadCreateJoin::handleKeyDown(int key, int action, bool repeat)
{
    if (m_ignoreInput || (!m_state19 && !m_customGame))
        return 0;
    if (action == 1)
    {
        m_activeList = 0;
        return 1;
    }
    if (action == 3 || action == 6)
    {
        m_currentButton++;
        if (m_currentButton >= 3)
            m_currentButton = 0;
        m_pendingAction = key;
    }
    repeat = repeat;
    return 0;
}

void IUIScene_LoadCreateJoin::LoadLevelGen(LevelGenerationOptions* options)
{
    if (options != 0)
    {
        m_levelGenerationOptions.push_back(options);
        m_state1a = 1;
    }
}

void IUIScene_LoadCreateJoin::LoadSaveFromDisk(File* file, ESavePlatform platform)
{
    if (file != 0)
    {
        m_saveData = static_cast<uint32_t>(platform);
        m_state1a = 1;
    }
}

void IUIScene_LoadCreateJoin::LoadGameMode(EMiniGameId gameMode)
{
    m_gameMode = static_cast<int>(gameMode);
    m_state1a = 1;
}

void IUIScene_LoadCreateJoin::HandleLoadListPress(int index)
{
    if (index >= 0 && index < static_cast<int>(m_buttonIds.size()))
    {
        m_selectedSave = index;
        m_pendingAction = 1;
    }
}

void IUIScene_LoadCreateJoin::HandleNewListPress(int index, int variant)
{
    m_selectedGame = index;
    m_gameMode = variant;
    m_pendingAction = 2;
}

void IUIScene_LoadCreateJoin::TexturePackDialogReturned(void* context, int userIndex,
                                                         C4JStorage::EMessageResult result)
{
    IUIScene_LoadCreateJoin* scene = static_cast<IUIScene_LoadCreateJoin*>(context);
    if (scene != 0)
        scene->m_selectedTexturePack = result == C4JStorage::EMessageResult_5 ? userIndex : 0;
}

void IUIScene_LoadCreateJoin::CheckAndJoinGame(int index)
{
    if (index >= 0 && index < static_cast<int>(m_friendSessions.size()))
    {
        m_selectedGame = index;
        m_state1a = 1;
    }
}

void IUIScene_LoadCreateJoin::HandleGamesListPressed(int index)
{
    m_ignoreInput = true;
    m_activeList = 2;
    m_selectedGame = index;
    CheckAndJoinGame(index);
}

void IUIScene_LoadCreateJoin::SetIgnoreInput(bool ignore)
{
    m_ignoreInput = ignore;
}

