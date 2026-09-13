#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"

#include <cstring>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/ui/FJ_LoadCreateJoinMenu.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_SaveListDetails.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_LoadCreateJoinMenu::UIScene_LoadCreateJoinMenu(int sceneType, void* data, UILayer* layer)
    : IUIScene_LoadCreateJoin(), UIScene(sceneType, layer), m_selectedLoad(0), m_selectedCreate(0),
      m_selectedJoin(0), m_player(0), m_activeTab(0), m_matchesAvailable(0), m_hideLoadTab(0),
      m_loadThumbnailPending(0), m_createThumbnailPending(0), m_joinThumbnailPending(0),
      m_saveOperationPending(0), m_saveDeviceRemoved(0), m_initialised(0),
      m_focusChanged(0), m_thumbnailIndex(0), m_callbackId(0), m_saveDetailsCount(0)
{
    const bool subScene = data != 0;
    initialiseMovie();
    g_pApp->SetLiveLinkRequired(true);
    m_loadTimer.setVisible(true);
    m_createTimer.setVisible(false);
    m_joinTimer.setVisible(true);
    gStorage->ClearSavesInfo();

    FJ_LoadCreateJoinMenu* menu = static_cast<FJ_LoadCreateJoinMenu*>(
        m_movie->getRootNode()->getFuiNode());
    menu->ShowSaveSizeBar(!subScene);
    m_loadList.init(0);
    m_createList.init(1);
    m_gamesList.init(2);
    _init(subScene);
    m_saveSizeBar.init(UIString(L""), 3, 0LL, 0x1e000000LL);
    if (!subScene) {
        m_loadTitle.init(UIString(0xe63a8eca));
        m_createTitle.init(UIString(0x41716830));
        m_joinTitle.init(UIString(0xa1b54cbe));
    } else {
        m_loadTitle.init(UIString(0x41716830));
        m_createTitle.init(UIString(0xa1b54cbe));
        m_joinTitle.init(UIString(0xdab3c895));
    }
    m_loadTouch.init(4);
    m_createTouch.init(5);
    m_joinTouch.init(6);
    gConsoleUIController.TouchBoxRebuild(this);
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
    fuiRenderNode* root = m_movie->getRootNode();

    m_loadTab.setupControl(this, root, "LoadGame");
    m_controls.push_back(&m_loadTab);
    fuiRenderNode* loadRoot = m_loadTab.getFuiRenderNode();
    m_loadList.setupControl(this, loadRoot, "LoadGameList");
    m_controls.push_back(&m_loadList);
    m_loadTimer.setupControl(this, loadRoot, "LoadGameTimer");
    m_controls.push_back(&m_loadTimer);
    m_saveSizeBar.setupControl(this, loadRoot, "SaveSizeBar");
    m_controls.push_back(&m_saveSizeBar);
    m_loadTouch.setupControl(this, loadRoot, "TouchTabLoad");
    m_controls.push_back(&m_loadTouch);

    m_createTab.setupControl(this, root, "NewGame");
    m_controls.push_back(&m_createTab);
    fuiRenderNode* createRoot = m_createTab.getFuiRenderNode();
    m_createList.setupControl(this, createRoot, "NewGameList");
    m_controls.push_back(&m_createList);
    m_createTimer.setupControl(this, createRoot, "NewGameTimer");
    m_controls.push_back(&m_createTimer);
    m_createTouch.setupControl(this, createRoot, "TouchTabCreate");
    m_controls.push_back(&m_createTouch);

    m_joinTab.setupControl(this, root, "JoinGame");
    m_controls.push_back(&m_joinTab);
    fuiRenderNode* joinRoot = m_joinTab.getFuiRenderNode();
    m_gamesList.setupControl(this, joinRoot, "JoinGameGamesList");
    m_controls.push_back(&m_gamesList);
    m_joinTimer.setupControl(this, joinRoot, "JoinGamesTimer");
    m_controls.push_back(&m_joinTimer);
    m_joinTouch.setupControl(this, joinRoot, "TouchTabJoin");
    m_controls.push_back(&m_joinTouch);

    m_loadTitle.setupControl(this, root, "LoadGameTabTitle");
    m_controls.push_back(&m_loadTitle);
    m_createTitle.setupControl(this, root, "NewGameTabTitle");
    m_controls.push_back(&m_createTitle);
    m_joinTitle.setupControl(this, root, "JoinGameTabTitle");
    m_controls.push_back(&m_joinTitle);
    return 1;
}

std::wstring UIScene_LoadCreateJoinMenu::getThumbnailName(int index)
{
    if (m_saveListDetails == 0 || index < 0 || index >= m_saveDetailsCount)
        return std::wstring();
    wchar_t name[33];
    for (unsigned int i = 0; i < 32; ++i)
        name[i] = static_cast<unsigned char>(m_saveListDetails[index].m_filename[i]);
    name[32] = L'\0';
    return std::wstring(name);
}

int UIScene_LoadCreateJoinMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (menu != 0)
    {
        _SaveListDetails& details = menu->m_saveListDetails[menu->m_state2c];
        if (data == 0 || size == 0)
        {
            details.m_thumbnail = 0;
            details.m_thumbnailSize = 0;
        }
        else
        {
            details.m_thumbnail = new unsigned char[size];
            std::memcpy(details.m_thumbnail, data, size);
            details.m_thumbnailSize = size;
        }
        menu->m_loadThumbnailPending = 1;
    }
    return 0;
}

void UIScene_LoadCreateJoinMenu::requestNextThumbnail()
{
    if (m_state2c < static_cast<int>(m_loadList.getItemCount()) &&
        m_state2c < static_cast<int>(m_thumbnailIndex) + 0x15)
    {
        SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
        if (details != 0 && m_state2c < details->count)
        {
            int result = gStorage->LoadSaveDataThumbnail(&details->saves[m_state2c],
                                                          &LoadSaveDataThumbnailReturned, this);
            if (result == 0x15)
                return;
        }
    }
    m_state1a = 0;
    m_state1c = 1;
}

void UIScene_LoadCreateJoinMenu::cleanUpThumbNails(bool all)
{
    if (m_customGame != 0)
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
        if (m_saveListDetails[m_state2c].m_thumbnail == 0)
            continue;

        if (hasRegisteredSubstitutionTexture(thumbnailName))
        {
            unregisterSubstitutionTexture(thumbnailName, false);
            m_loadList.setTextureName(index, std::wstring(L""));
        }
    }
}

void UIScene_LoadCreateJoinMenu::tick()
{
    UIScene::tick();
    if (m_state1d != 0 && m_state1a == 0)
        navigateBack();
    if (hasFocus(m_playerId))
    {
        EnterCriticalSection(&m_lock);
        if (m_state7c != 0)
        {
            gConsoleUIController.NavigateToHomeMenu(false);
            m_state7c = 0;
        }
        LeaveCriticalSection(&m_lock);
        if (m_focusChanged != 0)
        {
            m_saveSizeBar.selectSave(DoesLoadListHaveFocus() ? m_state3c : -1);
            m_focusChanged = 0;
        }
        if (m_state1b == 0)
        {
            m_saveDetails = gStorage->ReturnSavesInfo();
            if (m_saveDetails != 0)
            {
                m_saveSizeBar.reset();
                int savedCreateSelection = m_state44;
                AddDefaultButtons();
                m_state44 = savedCreateSelection;
                if (m_activeTab == 1)
                {
                    m_createList.setCurrentSelection(m_state44, false, true);
                    m_createList.updateChildFocus(m_state44);
                }
                m_state1b = 1;
                UpdateGamesList();
                if (m_customGame == 0)
                {
                    if (m_saveListDetails != 0)
                    {
                        for (int i = 0; i < m_saveDetailsCount; ++i)
                        {
                            std::wstring name = getThumbnailName(i);
                            if (hasRegisteredSubstitutionTexture(name))
                                unregisterSubstitutionTexture(name, false);
                            delete[] m_saveListDetails[i].m_thumbnail;
                        }
                        delete[] m_saveListDetails;
                    }
                    m_saveDetailsCount = m_saveDetails->count;
                    m_saveListDetails = m_saveDetailsCount > 0 ? new _SaveListDetails[m_saveDetailsCount] : 0;
                    int savedLoadSelection = m_state3c;
                    for (int i = 0; i < m_saveDetailsCount; ++i)
                    {
                        SAVE_INFO& save = m_saveDetails->saves[i];
                        m_saveSizeBar.addSave(save.nandSize);
                        wchar_t title[128];
                        std::memset(title, 0, sizeof(title));
                        std::memcpy(title, save.title, sizeof(title));
                        m_loadList.addItem(std::wstring(title), std::wstring(L""));
                        m_saveListDetails[i].m_value0 = i;
                        std::memcpy(m_saveListDetails[i].m_title, save.title, sizeof(m_saveListDetails[i].m_title));
                        std::memcpy(m_saveListDetails[i].m_filename, save.filename, sizeof(m_saveListDetails[i].m_filename));
                    }
                    m_state3c = savedLoadSelection;
                    if (m_activeTab == 0)
                    {
                        m_loadList.setCurrentSelection(m_state3c, false, true);
                        m_loadList.updateChildFocus(m_state3c);
                    }
                    ShowSaveListTimer(false);
                    updateTooltips();
                }
                else
                {
                    m_state1a = 0;
                    m_state1c = 1;
                }
            }
        }
        if (m_state1b != 0)
        {
            if (m_state1d == 0 && m_state1a == 0 && m_state1c == 0)
            {
                m_state1a = 1;
                requestNextThumbnail();
                _tick();
                return;
            }
            if (m_loadThumbnailPending != 0)
            {
                m_loadThumbnailPending = 0;
                if (m_state1d == 0)
                {
                    std::wstring name = getThumbnailName(m_state2c);
                    if (m_saveListDetails[m_state2c].m_thumbnail != 0 &&
                        !hasRegisteredSubstitutionTexture(name))
                        registerSubstitutionTexture(name, m_saveListDetails[m_state2c].m_thumbnail, m_saveListDetails[m_state2c].m_thumbnailSize, false);
                    m_loadList.setTextureName(m_state2c, name);
                    ++m_state2c;
                    requestNextThumbnail();
                    _tick();
                    return;
                }
                m_state1a = 0;
                _tick();
                return;
            }
        }
        int itemCount = static_cast<int>(m_loadList.getItemCount());
        if (itemCount > 0x15)
        {
            int top = static_cast<int>(m_loadList.GetTopItem()) - 7;
            if (top < 0)
                top = 0;
            if (top != static_cast<int>(m_thumbnailIndex))
            {
                m_state2c = top;
                m_loadThumbnailPending = 1;
                m_thumbnailIndex = top;
                cleanUpThumbNails(false);
            }
        }
    }
    _tick();
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
    m_layer->showComponent(m_player, static_cast<EUIScene>(66), true);
    m_layer->showComponent(m_player, static_cast<EUIScene>(67), false);
}

void UIScene_LoadCreateJoinMenu::handleGainFocus(bool hadFocus)
{
    UIScene::handleGainFocus(hadFocus);
    addTimer(0, 100);
    if (hadFocus)
    {
        g_pApp->SetLiveLinkRequired(true);
        m_ignoreInput = 0;
        m_state19 = (gProfile->IsSignedIn(m_playerId) &&
                     gProfile->IsSignedInLive(m_playerId)) ? 1 : 0;
        if (g_pApp->StartInstallDLCProcess(m_playerId) != 0 || g_pApp->DLCInstallPending() != 0)
        {
            m_ignoreInput = 1;
            ClearLoadList();
            ShowSaveListTimer(true);
        }
        if (m_state19 == 0)
        {
            handleReload();
            ShowJoinListTimer(true);
            ShowNoGamesLabel(false);
        }
        if (g_pApp->GetCorruptSaveDeleted() != 0)
        {
            m_state28 = 3;
            g_pApp->SetCorruptSaveDeleted(false);
        }
    }
}

void UIScene_LoadCreateJoinMenu::handleLoseFocus()
{
    killTimer(0);
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
    if (m_ignoreInput != 0)
        return;
    if (m_customGame == 0 && m_state1b == 0)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (!released)
        return;
    if (action == 4 || action == 5 || action == 8 || action == 9 || action == 0x14)
    {
        if (action == 0x14)
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
    }
    else
        handled = handleKeyDown(m_playerId, action, repeat) != 0;
}

void UIScene_LoadCreateJoinMenu::handleDestroy()
{
    destroyMovie();
}

void UIScene_LoadCreateJoinMenu::handleReload()
{
    _init(m_customGame != 0);
    reloadMovie(true);
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
        if (m_customGame == 0)
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
    _init(m_customGame != 0);
}

void UIScene_LoadCreateJoinMenu::HandleDLCInstalled()
{
    g_pApp->StartInstallDLCProcess(m_player);
}

void UIScene_LoadCreateJoinMenu::HandleSaveDeviceRemoved(int)
{
    if (m_customGame == 0)
        m_saveDeviceRemoved = 1;
}

void UIScene_LoadCreateJoinMenu::handleTouchInput(unsigned int playerId, int action, int x, int y,
                                                  bool repeat, bool released, bool pressed)
{
    UIScene::handleTouchInput(playerId, action, x, y, repeat, released, pressed);
}

void UIScene_LoadCreateJoinMenu::SelectTab(int tab)
{
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    if (m_hideLoadTab != 0)
        tab = tab == 0 ? 1 : 2;
    FJ_LoadCreateJoinMenu* menu = static_cast<FJ_LoadCreateJoinMenu*>(
        m_movie->getRootNode()->getFuiNode());
    menu->SetActiveTab(tab);
    gConsoleUIController.TouchBoxRebuild(this);
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
    m_loadList.addItem(label);
}

void UIScene_LoadCreateJoinMenu::SetLoadListIcon(int, unsigned char* data, unsigned int size,
                                                 const std::wstring& name)
{
    registerSubstitutionTexture(name, data, size, false);
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
    m_createList.addItem(label);
}

bool UIScene_LoadCreateJoinMenu::DoesCreateListHaveFocus()
{
    return m_createList.hasFocus();
}

void UIScene_LoadCreateJoinMenu::SetCreateListIcon(int, unsigned char* data, unsigned int size,
                                                   const std::wstring& name)
{
    registerSubstitutionTexture(name, data, size, false);
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
        registerSubstitutionTexture(textureName, data, size, false);
    if (data1 != 0 && size1 != 0)
        registerSubstitutionTexture(textureName1, data1, size1, false);
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
    m_state34 = 0;
    m_state38 = 0;
    m_state3c = 0;
    m_state40 = 0;
    m_state44 = 0;
    if (!gProfile->IsFullVersion() || m_customGame != 0)
    {
        AddDefaultButtons();
        m_ignoreInput = 0;
    }
    else
    {
        if (gStorage->GetSaveDisabled() == 0)
            gStorage->EnoughSpaceForAMinSaveGame();
        m_state28 = 1;
    }
    gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
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
    return m_customGame;
}

int UIScene_LoadCreateJoinMenu::GetPad()
{
    return m_playerId;
}
