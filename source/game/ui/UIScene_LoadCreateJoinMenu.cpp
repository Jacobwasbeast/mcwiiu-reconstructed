#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"

#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/ui/FJ_LoadCreateJoinMenu.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UILayer.hpp"
#include "vendor/_LoadingInputParams.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

namespace
{
unsigned char& baseIgnoreInput(IUIScene_LoadCreateJoin* scene)
{
    return *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(scene) + 0x20);
}

char*& baseSaveDataBase(IUIScene_LoadCreateJoin* scene)
{
    return *reinterpret_cast<char**>(reinterpret_cast<char*>(scene) + 0xc);
}

int& baseSaveDataIndex(IUIScene_LoadCreateJoin* scene)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(scene) + 0x2c);
}
}

UIScene_LoadCreateJoinMenu::UIScene_LoadCreateJoinMenu(int sceneType, void*, UILayer* layer)
    : IUIScene_LoadCreateJoin(), m_scene(sceneType, layer), m_selectedLoad(0), m_selectedCreate(0),
      m_selectedJoin(0), m_player(0), m_activeTab(0), m_matchesAvailable(0), m_hideLoadTab(0),
      m_loadThumbnailPending(0), m_createThumbnailPending(0), m_joinThumbnailPending(0),
      m_saveOperationPending(0), m_subSceneType(0), m_saveDeviceRemoved(0), m_initialised(0),
      m_focusChanged(0), m_thumbnailIndex(0), m_callbackId(0)
{
}

UIScene_LoadCreateJoinMenu::~UIScene_LoadCreateJoinMenu()
{
}

std::wstring UIScene_LoadCreateJoinMenu::getMoviePath()
{
    return std::wstring(L"LoadCreateJoinMenu");
}

unsigned int UIScene_LoadCreateJoinMenu::mapElementsAndNames()
{
    m_loadList.setupControl(&m_scene, 0, "LoadGameList");
    m_createList.setupControl(&m_scene, 0, "NewGameList");
    m_gamesList.setupControl(&m_scene, 0, "JoinGameGamesList");
    m_loadTouch.setupControl(&m_scene, 0, "TouchTabLoad");
    m_createTouch.setupControl(&m_scene, 0, "TouchTabCreate");
    m_joinTouch.setupControl(&m_scene, 0, "TouchTabJoin");
    m_loadTimer.setupControl(&m_scene, 0, "LoadGameTimer");
    m_createTimer.setupControl(&m_scene, 0, "NewGameTimer");
    m_joinTimer.setupControl(&m_scene, 0, "JoinGamesTimer");
    m_loadTitle.setupControl(&m_scene, 0, "LoadGameTabTitle");
    m_createTitle.setupControl(&m_scene, 0, "NewGameTabTitle");
    m_joinTitle.setupControl(&m_scene, 0, "JoinGameTabTitle");
    m_controls.push_back(&m_loadList);
    m_controls.push_back(&m_createList);
    m_controls.push_back(&m_gamesList);
    return 1;
}

std::wstring UIScene_LoadCreateJoinMenu::getThumbnailName(int)
{
    return std::wstring();
}

uint32_t UIScene_LoadCreateJoinMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char*, unsigned int)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (menu != 0)
        menu->m_loadThumbnailPending = 1;
    return 0;
}

void UIScene_LoadCreateJoinMenu::requestNextThumbnail()
{
    m_loadThumbnailPending = 0;
}

void UIScene_LoadCreateJoinMenu::cleanUpThumbNails(bool all)
{
    if (baseIgnoreInput(this) != 0)
        return;
    if (g_pApp->DebugSettingsOn() != 0 && g_pApp->GetLoadSavesFromFolderEnabled() != 0)
        return;

    int count = static_cast<int>(m_loadList.getItemCount());
    for (int index = 0; index < count; ++index, count = static_cast<int>(m_loadList.getItemCount()))
    {
        if (!(all || index < static_cast<int>(m_thumbnailIndex) ||
              static_cast<int>(m_thumbnailIndex) + 0x15 <= index))
            continue;

        std::wstring thumbnailName = getThumbnailName(index);
        if (*reinterpret_cast<int*>(baseSaveDataBase(this) + baseSaveDataIndex(this) * 300 + 4) == 0)
            continue;

        if (m_scene.hasRegisteredSubstitutionTexture(thumbnailName))
        {
            m_scene.unregisterSubstitutionTexture(thumbnailName, false);
            reinterpret_cast<UIControl_SaveList&>(m_loadList).setTextureName(index, std::wstring());
        }
    }
}

void UIScene_LoadCreateJoinMenu::tick()
{
    m_scene.tick();
}

void UIScene_LoadCreateJoinMenu::handleTimerComplete(int timerId)
{
    handleTimer(timerId);
}

void UIScene_LoadCreateJoinMenu::updateTooltips()
{
    _updateTooltips();
}

void UIScene_LoadCreateJoinMenu::updateComponents()
{
    if (m_scene.getSceneType() != 0)
        m_scene.setVisible(true);
}

void UIScene_LoadCreateJoinMenu::handleGainFocus(bool)
{
    m_scene.addTimer(0, 100);
    m_initialised = 1;
}

void UIScene_LoadCreateJoinMenu::handleLoseFocus()
{
    m_scene.killTimer(0);
}

UIControl* UIScene_LoadCreateJoinMenu::GetMainPanel()
{
    if (m_activeTab == 1)
        return &m_createTab;
    if (m_activeTab == 2)
        return &m_joinTab;
    return &m_loadTab;
}

void UIScene_LoadCreateJoinMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                             bool pressed, bool& handled)
{
    if (m_saveOperationPending != 0)
        return;
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (released)
        handled = handleKeyDown(playerId, action, repeat) != 0;
    else
        m_scene.sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_LoadCreateJoinMenu::handleDestroy()
{
    m_scene.destroyMovie();
}

void UIScene_LoadCreateJoinMenu::handleReload()
{
    _init(m_subSceneType != 0);
    m_scene.reloadMovie(true);
}

void UIScene_LoadCreateJoinMenu::handlePress(int first, int second)
{
    if (first == 0)
    {
        HandleLoadListPress(second);
        return;
    }
    if (first == 2)
    {
        HandleGamesListPressed(second);
        return;
    }
    if (first == 1)
    {
        int child = second;
        if (m_scene.getSceneType() == 0)
            --child;
        HandleNewListPress(second, child);
    }
}

void UIScene_LoadCreateJoinMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
    {
        m_focusChanged = 1;
        m_selectedLoad = childFocus;
    }
    else if (focus == 1)
        m_selectedCreate = childFocus;
    else if (focus == 2)
    {
        m_selectedJoin = childFocus;
        m_gamesList.updateChildFocus(childFocus);
    }
}

void UIScene_LoadCreateJoinMenu::handleInitFocus(int, int)
{
}

void UIScene_LoadCreateJoinMenu::HandleDLCMountingComplete()
{
    _init(m_subSceneType != 0);
}

void UIScene_LoadCreateJoinMenu::HandleDLCInstalled()
{
    g_pApp->StartInstallDLCProcess(m_player);
}

void UIScene_LoadCreateJoinMenu::HandleSaveDeviceRemoved(int)
{
    if (m_subSceneType == 0)
        m_saveDeviceRemoved = 1;
}

void UIScene_LoadCreateJoinMenu::handleTouchInput(unsigned int playerId, int action, int x, int y,
                                                  bool repeat, bool released, bool pressed)
{
    m_scene.handleTouchInput(playerId, action, x, y, repeat, released, pressed);
}

void UIScene_LoadCreateJoinMenu::SelectTab(int tab)
{
    m_activeTab = tab;
}

void UIScene_LoadCreateJoinMenu::SetMatchesAvailable(bool available)
{
    m_matchesAvailable = available ? 1 : 0;
}

void UIScene_LoadCreateJoinMenu::SetHideLoadTab()
{
    m_hideLoadTab = 1;
}

bool UIScene_LoadCreateJoinMenu::GetHideLoadTab()
{
    return m_hideLoadTab != 0;
}

void UIScene_LoadCreateJoinMenu::AddLoadListButton(const std::wstring& label, int id)
{
    m_loadList.addItem(label, id);
}

void UIScene_LoadCreateJoinMenu::SetLoadListIcon(int, unsigned char* data, unsigned int size,
                                                 const std::wstring& name)
{
    m_scene.registerSubstitutionTexture(name, data, size, false);
}

void UIScene_LoadCreateJoinMenu::ScrollLoadListToSelection(unsigned int selection)
{
    m_loadList.setCurrentSelection(selection, false, true);
}

void UIScene_LoadCreateJoinMenu::ClearLoadList()
{
    cleanUpThumbNails(true);
    m_loadList.clearList();
}

unsigned int UIScene_LoadCreateJoinMenu::GetLoadListSize()
{
    return m_loadList.getItemCount();
}

unsigned int UIScene_LoadCreateJoinMenu::GetLoadListSelection()
{
    return m_loadList.getCurrentSelection();
}

void UIScene_LoadCreateJoinMenu::SetLoadListEnabled(bool enabled)
{
    m_loadList.setVisible(enabled);
}

void UIScene_LoadCreateJoinMenu::AddCreateListButton(const std::wstring& label, int id)
{
    m_createList.addItem(label, id);
}

bool UIScene_LoadCreateJoinMenu::DoesCreateListHaveFocus()
{
    return m_createList.hasFocus();
}

void UIScene_LoadCreateJoinMenu::SetCreateListIcon(int, unsigned char* data, unsigned int size,
                                                   const std::wstring& name)
{
    m_scene.registerSubstitutionTexture(name, data, size, false);
}

void UIScene_LoadCreateJoinMenu::ScrollCreateListToSelection(unsigned int selection)
{
    m_createList.setCurrentSelection(selection, false, true);
}

void UIScene_LoadCreateJoinMenu::ClearCreateList()
{
    m_createList.clearList();
}

void UIScene_LoadCreateJoinMenu::SetCreateListEnabled(bool enabled)
{
    m_createList.setVisible(enabled);
}

bool UIScene_LoadCreateJoinMenu::AddJoinListButton(const std::wstring& label, int id, unsigned char* data,
                                                   unsigned int size, const std::wstring& textureName,
                                                   unsigned char* data1, unsigned int size1,
                                                   const std::wstring& textureName1)
{
    if (data != 0 && size != 0)
        m_scene.registerSubstitutionTexture(textureName, data, size, false);
    if (data1 != 0 && size1 != 0)
        m_scene.registerSubstitutionTexture(textureName1, data1, size1, false);
    m_gamesList.AddNewItem(label, id, textureName1, textureName);
    return true;
}

void UIScene_LoadCreateJoinMenu::InitFocusJoinList(int focus)
{
    m_gamesList.updateChildFocus(focus);
}

bool UIScene_LoadCreateJoinMenu::ClearJoinList()
{
    m_gamesList.clearList();
    return true;
}

unsigned int UIScene_LoadCreateJoinMenu::GetJoinListSize()
{
    return m_gamesList.getItemCount();
}

unsigned int UIScene_LoadCreateJoinMenu::GetJoinListSelection()
{
    return m_gamesList.getCurrentSelection();
}

bool UIScene_LoadCreateJoinMenu::IsJoinListEnabled()
{
    return true;
}

void UIScene_LoadCreateJoinMenu::SetJoinListEnabled(bool enabled)
{
    m_gamesList.setVisible(enabled);
}

void UIScene_LoadCreateJoinMenu::SetGamesListSelection(unsigned int selection)
{
    m_gamesList.setCurrentSelection(selection, false, true);
}

void UIScene_LoadCreateJoinMenu::ShowSaveListTimer(bool show)
{
    m_loadTimer.setVisible(show);
}

void UIScene_LoadCreateJoinMenu::ShowJoinListTimer(bool show)
{
    m_joinTimer.setVisible(show);
}

void UIScene_LoadCreateJoinMenu::ShowNoGamesLabel(bool show)
{
    m_joinTitle.setVisible(show);
}

uint32_t UIScene_LoadCreateJoinMenu::DeleteSaveDataReturned(void* context, bool success)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (menu != 0)
        menu->m_saveOperationPending = success ? 0 : 1;
    return 0;
}

void UIScene_LoadCreateJoinMenu::DeleteSaveFile(int)
{
    m_saveOperationPending = 1;
}

void UIScene_LoadCreateJoinMenu::Initialise()
{
    m_initialised = 1;
}

bool UIScene_LoadCreateJoinMenu::DoesLoadListHaveFocus()
{
    return m_loadList.hasFocus();
}

bool UIScene_LoadCreateJoinMenu::DoesMashUpWorldHaveFocus()
{
    return false;
}

bool UIScene_LoadCreateJoinMenu::DoesGamesListHaveFocus()
{
    return m_gamesList.hasFocus();
}

uint32_t UIScene_LoadCreateJoinMenu::RenameSaveDataReturned(void* context, bool success)
{
    return DeleteSaveDataReturned(context, success);
}

uint32_t UIScene_LoadCreateJoinMenu::KeyboardCompleteWorldNameCallback(void* context, bool complete)
{
    return RenameSaveDataReturned(context, complete);
}

uint32_t UIScene_LoadCreateJoinMenu::CopySaveErrorDialogFinishedCallback(void* context, int,
                                                                         C4JStorage::EMessageResult)
{
    return DeleteSaveDataReturned(context, false);
}

uint32_t UIScene_LoadCreateJoinMenu::CopySaveDataReturned(void* context, bool success,
                                                          C4JStorage::ESaveGameState)
{
    return DeleteSaveDataReturned(context, success);
}

bool UIScene_LoadCreateJoinMenu::CopySaveDataProgress(void*, int progress)
{
    return progress >= 0;
}

uint32_t UIScene_LoadCreateJoinMenu::CopySaveThreadProc(void* context)
{
    return CopySaveDataReturned(context, true, static_cast<C4JStorage::ESaveGameState>(0));
}

uint32_t UIScene_LoadCreateJoinMenu::CopySaveDialogReturned(void* context, int,
                                                            C4JStorage::EMessageResult)
{
    return DeleteSaveDataReturned(context, true);
}

uint32_t UIScene_LoadCreateJoinMenu::LoadSaveDataForUploadReturned(void* context, bool success, bool)
{
    return DeleteSaveDataReturned(context, success);
}

uint32_t UIScene_LoadCreateJoinMenu::CrossSaveUploadFinishedCallback(void* context, int userIndex,
                                                                      C4JStorage::EMessageResult result)
{
    return CopySaveDialogReturned(context, userIndex, result);
}

uint32_t UIScene_LoadCreateJoinMenu::UploadNintendoLanSaveThreadProc(void*)
{
    return 0;
}

void UIScene_LoadCreateJoinMenu::CancelSaveUploadCallback(void* context)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (menu != 0)
        menu->m_saveOperationPending = 0;
}

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
}

void UIScene_LoadCreateJoinMenu::LaunchSaveUpload()
{
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_firstParameter = reinterpret_cast<void* (*)(void*)>(&UploadNintendoLanSaveThreadProc);
    params->m_secondParameter = this;

    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->unknownState1 = true;
    progressState->unknownState2 = true;
    progressState->showTip = true;
    progressState->closeMode = 6;
    progressState->playerId = 0;
    progressState->backScene = static_cast<EUIScene>(0x53);
    params->m_thirdParameter = progressState;

    params->m_selectedIndex = static_cast<int32_t>(0xe21d8023u);
    params->m_fifthParameter = reinterpret_cast<int32_t>(&CancelSaveUploadCallback);
    params->m_seventhParameter = reinterpret_cast<int32_t>(this);

    gConsoleUIController.NavigateToScene(m_player, static_cast<EUIScene>(4), params,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}

void UIScene_LoadCreateJoinMenu::SaveTransferDialogReturned(void* context, int,
                                                             C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(4))
        static_cast<UIScene_LoadCreateJoinMenu*>(context)->LaunchSaveUpload();
}

void UIScene_LoadCreateJoinMenu::handleSaveOptionsDialogReturned(int,
                                                                 C4JStorage::EMessageResult result)
{
    if (result == static_cast<C4JStorage::EMessageResult>(4))
        m_saveOperationPending = 1;
    else if (result < static_cast<C4JStorage::EMessageResult>(4))
        m_saveOperationPending = 0;
}

void UIScene_LoadCreateJoinMenu::handleTexturePackDialogReturned(int,
                                                                  C4JStorage::EMessageResult)
{
    m_saveOperationPending = 0;
}

int UIScene_LoadCreateJoinMenu::getSceneType()
{
    return 27;
}

unsigned int UIScene_LoadCreateJoinMenu::getSubSceneType() const
{
    return m_subSceneType;
}

int UIScene_LoadCreateJoinMenu::GetPad()
{
    return m_player;
}
