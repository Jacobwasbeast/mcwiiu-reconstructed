#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/FJ_LoadCreateJoinMenu.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/NQRNetworkManager_Lan_WiiU.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_SaveListDetails.hpp"

extern CConsoleMinecraftApp gApp;
extern ConsoleUIController gConsoleUIController;
extern CInput* g_pInput;
extern CProfile* gProfile;
extern CStorage* gStorage;

namespace
{
const unsigned int THUMBNAIL_WINDOW_SIZE = 21;
const int THUMBNAIL_WINDOW_LEAD = 7;
const unsigned int UPLOAD_POLL_INTERVAL = 50;
const unsigned int COPY_POLL_INTERVAL = 100;

}

UIScene_LoadCreateJoinMenu::UIScene_LoadCreateJoinMenu(int playerId, void* data, UILayer* layer)
    : IUIScene_LoadCreateJoin(), UIScene(playerId, layer), m_saveDetailsCount(0), m_hideLoadTab(false),
      m_miniGamesMenu(data != NULL), m_saveDeviceRemoved(false), m_thumbnailWindowStart(0),
      m_thumbnailReturned(false), m_unusedCounter(0), m_copyInProgress(false), m_copyCancelled(false),
      m_unusedFlagA(false), m_unusedFlagB(false), m_loadSelectionChanged(false), m_activeTab(0),
      m_uploadState(eUploadState_Finished), m_uploadCancelled(false)
{
    initialiseMovie();
    gApp.SetLiveLinkRequired(true);
    m_loadTimer.setVisible(true);
    m_createTimer.setVisible(false);
    m_joinTimer.setVisible(true);
    gStorage->ClearSavesInfo();
    static_cast<FJ_LoadCreateJoinMenu*>(m_movie->getRootNode()->getFuiNode())->ShowSaveSizeBar(!m_miniGamesMenu);
    m_loadList.init(0);
    m_createList.init(1);
    m_gamesList.init(2);
    _init(data != NULL);
    m_saveSizeBar.init(UIString(L""), 3, 0, SAVE_DATA_QUOTA);
    if (!m_miniGames)
    {
        m_loadTitle.init(UIString(IDS_LOAD_TAB));
        m_createTitle.init(UIString(IDS_CREATE_TAB));
        m_joinTitle.init(UIString(IDS_JOIN_TAB));
    }
    else
    {
        m_loadTitle.init(UIString(IDS_CREATE_TAB));
        m_createTitle.init(UIString(IDS_JOIN_TAB));
        m_joinTitle.init(UIString(IDS_FRIENDS_TAB));
    }
    m_loadTouch.init(4);
    m_createTouch.init(5);
    m_joinTouch.init(6);
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_LoadCreateJoinMenu::~UIScene_LoadCreateJoinMenu()
{
    CGameNetworkManager::sInstance.SetSessionsUpdatedCallback(NULL, NULL);
    gApp.SetLiveLinkRequired(false);
    if (m_saveListDetails != NULL)
    {
        for (int i = 0; i < m_saveDetailsCount; ++i)
            delete m_saveListDetails[i].m_thumbnail;
        delete[] m_saveListDetails;
    }
}

std::wstring UIScene_LoadCreateJoinMenu::getMoviePath()
{
    return L"LoadCreateJoinMenu";
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

wchar_t* UIScene_LoadCreateJoinMenu::getThumbnailName(int index)
{
    wchar_t* name = new wchar_t[32];
    charTowchar_tArray(m_saveListDetails[index].m_filename, name, 32);
    return name;
}

int UIScene_LoadCreateJoinMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (data == NULL || size == 0)
    {
        menu->m_thumbnailReturned = true;
        menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnail = NULL;
        menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnailSize = 0;
        return 0;
    }
    unsigned char* thumbnail = new unsigned char[size];
    menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnail = thumbnail;
    std::memcpy(thumbnail, data, size);
    menu->m_thumbnailReturned = true;
    menu->m_saveListDetails[menu->m_nextThumbnail].m_thumbnailSize = size;
    return 0;
}

void UIScene_LoadCreateJoinMenu::requestNextThumbnail()
{
    int nextThumbnail = m_nextThumbnail;
    if (nextThumbnail < static_cast<int>(m_loadList.getItemCount()) &&
        nextThumbnail < m_thumbnailWindowStart + static_cast<int>(THUMBNAIL_WINDOW_SIZE))
    {
        SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
        if (m_nextThumbnail < details->count &&
            gStorage->LoadSaveDataThumbnail(&details->saves[m_nextThumbnail], &LoadSaveDataThumbnailReturned,
                                            this) == C4JStorage::ESaveGame_GetThumbnail)
            return;
    }
    m_thumbnailRequested = false;
    m_thumbnailsComplete = true;
}

void UIScene_LoadCreateJoinMenu::cleanUpThumbNails(bool all)
{
    if (m_miniGames)
        return;
    if (gApp.DebugSettingsOn() && gApp.GetLoadSavesFromFolderEnabled())
        return;

    for (int index = 0; index < static_cast<int>(m_loadList.getItemCount()); ++index)
    {
        if (!all && index >= m_thumbnailWindowStart &&
            index < m_thumbnailWindowStart + static_cast<int>(THUMBNAIL_WINDOW_SIZE))
            continue;
        const wchar_t* name = getThumbnailName(index);
        if (m_saveListDetails[m_nextThumbnail].m_thumbnail == NULL)
            continue;
        if (hasRegisteredSubstitutionTexture(name))
        {
            unregisterSubstitutionTexture(name, false);
            m_loadList.setTextureName(index, L"");
        }
    }
}

void UIScene_LoadCreateJoinMenu::tick()
{
    UIScene::tick();
    if (m_closing && !m_thumbnailRequested)
        navigateBack();
    if (!hasFocus(m_playerId))
    {
        _tick();
        return;
    }

    EnterCriticalSection(&m_lock);
    if (m_returnToHomeMenu)
    {
        gConsoleUIController.NavigateToHomeMenu(false);
        m_returnToHomeMenu = false;
    }
    LeaveCriticalSection(&m_lock);

    if (m_loadSelectionChanged)
    {
        m_saveSizeBar.selectSave(DoesLoadListHaveFocus() ? m_selectedSaveIndex : -1);
        m_loadSelectionChanged = false;
    }

    if (!m_saveInfoReady)
    {
        m_saveDetails = gStorage->ReturnSavesInfo();
        if (m_saveDetails != NULL)
        {
            m_saveSizeBar.reset();
            int createSelection = m_selectedCreateIndex;
            AddDefaultButtons();
            m_selectedCreateIndex = createSelection;
            if (m_activeTab == 1)
            {
                m_createList.setCurrentSelection(createSelection, false, true);
                m_createList.updateChildFocus(m_selectedCreateIndex);
            }
            m_saveInfoReady = true;
            UpdateGamesList();
            if (!m_miniGames)
            {
                if (m_saveListDetails != NULL)
                {
                    for (int i = 0; i < m_saveDetailsCount; ++i)
                    {
                        const wchar_t* name = getThumbnailName(i);
                        if (hasRegisteredSubstitutionTexture(name))
                            unregisterSubstitutionTexture(name, false);
                        delete m_saveListDetails[i].m_thumbnail;
                    }
                    delete[] m_saveListDetails;
                }
                m_saveListDetails = m_saveDetails->count > 0 ? new _SaveListDetails[m_saveDetails->count] : NULL;
                int loadSelection = m_selectedSaveIndex;
                m_saveDetailsCount = m_saveDetails->count;
                for (int i = 0; i < m_saveDetails->count; ++i)
                {
                    SAVE_INFO& save = m_saveDetails->saves[i];
                    m_saveSizeBar.addSave(save.nandSize);
                    m_loadList.addItem(std::wstring(save.title), L"");
                    m_saveListDetails[i].m_saveIndex = i;
                    std::memcpy(m_saveListDetails[i].m_title, save.title, sizeof(save.title));
                    std::memcpy(m_saveListDetails[i].m_filename, save.filename, sizeof(save.filename));
                }
                m_selectedSaveIndex = loadSelection;
                if (m_activeTab == 0)
                {
                    m_loadList.setCurrentSelection(loadSelection, false, true);
                    m_loadList.updateChildFocus(m_selectedSaveIndex);
                }
                ShowSaveListTimer(false);
                updateTooltips();
            }
            else
            {
                m_thumbnailRequested = false;
                m_thumbnailsComplete = true;
            }
        }
    }

    if (m_saveInfoReady)
    {
        if (!m_closing && !m_thumbnailRequested && !m_thumbnailsComplete)
        {
            m_thumbnailRequested = true;
            requestNextThumbnail();
            _tick();
            return;
        }
        if (m_thumbnailReturned)
        {
            m_thumbnailReturned = false;
            if (m_closing)
            {
                m_thumbnailRequested = false;
                _tick();
                return;
            }
            const wchar_t* name = getThumbnailName(m_nextThumbnail);
            _SaveListDetails& details = m_saveListDetails[m_nextThumbnail];
            if (details.m_thumbnail != NULL)
            {
                if (!hasRegisteredSubstitutionTexture(name))
                    registerSubstitutionTexture(name, details.m_thumbnail, details.m_thumbnailSize, false);
            }
            m_loadList.setTextureName(m_nextThumbnail, name);
            ++m_nextThumbnail;
            requestNextThumbnail();
            _tick();
            return;
        }
    }

    if (static_cast<int>(m_loadList.getItemCount()) > static_cast<int>(THUMBNAIL_WINDOW_SIZE))
    {
        int windowStart = static_cast<int>(m_loadList.GetTopItem()) - THUMBNAIL_WINDOW_LEAD;
        if (windowStart < 0)
            windowStart = 0;
        if (windowStart != m_thumbnailWindowStart)
        {
            m_nextThumbnail = windowStart;
            m_thumbnailReturned = true;
            m_thumbnailWindowStart = windowStart;
            cleanUpThumbNails(false);
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
    if (hasFocus(m_playerId))
        _updateTooltips();
}

void UIScene_LoadCreateJoinMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_LoadCreateJoinMenu::handleGainFocus(bool navigatedBack)
{
    updateTooltips();
    addTimer(0, 100);
    if (!navigatedBack)
        return;

    gApp.SetLiveLinkRequired(true);
    m_ignoreInput = false;
    m_onlinePlayAllowed = gProfile->IsSignedInLive(m_playerId) && gProfile->AllowedToPlayMultiplayer(m_playerId);
    if (gApp.StartInstallDLCProcess(m_playerId) == 0 && gApp.DLCInstallPending() == 0)
    {
        m_ignoreInput = false;
    }
    else
    {
        m_ignoreInput = true;
        ClearLoadList();
        ShowSaveListTimer(true);
    }
    if (!m_onlinePlayAllowed)
    {
        ClearJoinList();
        ShowJoinListTimer(true);
        ShowNoGamesLabel(false);
    }
    if (gApp.GetCorruptSaveDeleted())
    {
        m_pendingAction = ePendingAction_ReloadSaveInfo;
        gApp.SetCorruptSaveDeleted(false);
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

void UIScene_LoadCreateJoinMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                             bool& handled)
{
    if (m_ignoreInput)
        return;
    if (!m_miniGames && !m_saveInfoReady)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (!pressed)
        return;

    switch (action)
    {
    case EControllerActions_MenuOk:
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    default:
        handled = handleKeyDown(m_playerId, action, repeat);
        break;
    }
}

void UIScene_LoadCreateJoinMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

void UIScene_LoadCreateJoinMenu::handleReload()
{
    int createSelection = m_selectedCreateIndex;
    int loadSelection = m_selectedSaveIndex;
    _init(m_miniGames);
    static_cast<FJ_LoadCreateJoinMenu*>(m_movie->getRootNode()->getFuiNode())->ShowSaveSizeBar(!m_miniGamesMenu);
    SelectTab(m_activeTab);
    m_selectedSaveIndex = loadSelection;
    m_selectedCreateIndex = createSelection;
    if (!m_miniGames)
        return;
    if (m_activeTab == 0)
    {
        m_loadList.setCurrentSelection(loadSelection, false, true);
        m_loadList.updateChildFocus(loadSelection);
    }
    else if (m_activeTab == 1)
    {
        m_createList.setCurrentSelection(createSelection, false, true);
        m_createList.updateChildFocus(m_selectedCreateIndex);
    }
}

void UIScene_LoadCreateJoinMenu::handlePress(int controlId, int childId)
{
    switch (controlId)
    {
    case 0:
        HandleLoadListPress(childId);
        break;
    case 1:
    {
        int createIndex = childId;
        if (!m_miniGames)
            createIndex = childId - 1;
        HandleNewListPress(childId, createIndex);
        break;
    }
    case 2:
        HandleGamesListPressed(childId);
        break;
    }
}

void UIScene_LoadCreateJoinMenu::handleFocusChange(int controlId, int childId)
{
    switch (controlId)
    {
    case 0:
        m_loadSelectionChanged = true;
        m_selectedSaveIndex = childId;
        break;
    case 1:
        m_selectedCreateIndex = childId;
        break;
    case 2:
        m_selectedGameIndex = childId;
        m_gamesList.updateChildFocus(childId);
        break;
    }
    updateTooltips();
}

void UIScene_LoadCreateJoinMenu::handleInitFocus(int, int)
{
}

void UIScene_LoadCreateJoinMenu::HandleDLCMountingComplete()
{
    Initialise();
}

void UIScene_LoadCreateJoinMenu::HandleDLCInstalled()
{
    gApp.StartInstallDLCProcess(m_playerId);
}

void UIScene_LoadCreateJoinMenu::HandleSaveDeviceRemoved(int)
{
    if (!m_miniGames)
        m_saveDeviceRemoved = true;
}

void UIScene_LoadCreateJoinMenu::handleTouchInput(unsigned int, int, int, int touchBox, bool pressed, bool, bool)
{
    unsigned int box = touchBox;
    if (!pressed)
        return;
    if (box < 4)
        return;
    if (box > 6)
        return;
    int tab = box - 4;
    if (GetHideLoadTab())
        tab = box - 5;
    if (m_activeTab == tab)
        return;
    m_activeTab = tab;
    updateTooltips();
    SelectTab(m_activeTab);
}

void UIScene_LoadCreateJoinMenu::SelectTab(int tab)
{
    m_activeTab = tab;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    if (m_hideLoadTab)
        tab = tab == 0 ? 1 : 2;
    static_cast<FJ_LoadCreateJoinMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(tab);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_LoadCreateJoinMenu::SetMatchesAvailable(bool available)
{
    static_cast<FJ_LoadCreateJoinMenu*>(m_movie->getRootNode()->getFuiNode())->SetMatchesAvailable(available);
}

void UIScene_LoadCreateJoinMenu::SetHideLoadTab()
{
    static_cast<FJ_LoadCreateJoinMenu*>(m_movie->getRootNode()->getFuiNode())->HideLoadTab();
    m_hideLoadTab = true;
}

bool UIScene_LoadCreateJoinMenu::GetHideLoadTab()
{
    return m_hideLoadTab;
}

void UIScene_LoadCreateJoinMenu::AddLoadListButton(const std::wstring& label, int)
{
    m_loadList.addItem(label);
}

void UIScene_LoadCreateJoinMenu::SetLoadListIcon(int index, unsigned char* data, unsigned int size,
                                                 const std::wstring& name)
{
    registerSubstitutionTexture(name, data, size, false);
    m_loadList.setTextureName(index, name);
}

void UIScene_LoadCreateJoinMenu::ScrollLoadListToSelection(unsigned int)
{
    m_loadList.setCurrentSelection(0, false, true);
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

void UIScene_LoadCreateJoinMenu::SetLoadListEnabled(bool)
{
}

void UIScene_LoadCreateJoinMenu::AddCreateListButton(const std::wstring& label, int)
{
    m_createList.addItem(label);
}

bool UIScene_LoadCreateJoinMenu::DoesCreateListHaveFocus()
{
    return m_createList.hasFocus();
}

void UIScene_LoadCreateJoinMenu::SetCreateListIcon(int index, unsigned char* data, unsigned int size,
                                                   const std::wstring& name)
{
    registerSubstitutionTexture(name, data, size, false);
    m_createList.setTextureName(index, name);
}

void UIScene_LoadCreateJoinMenu::ScrollCreateListToSelection(unsigned int)
{
    m_createList.setCurrentSelection(0, false, true);
}

void UIScene_LoadCreateJoinMenu::ClearCreateList()
{
    m_createList.clearList();
}

void UIScene_LoadCreateJoinMenu::SetCreateListEnabled(bool)
{
}

void UIScene_LoadCreateJoinMenu::AddJoinListButton(const std::wstring& label, int id, unsigned char* gameIcon,
                                                   unsigned int gameIconSize, const std::wstring& gameIconName,
                                                   unsigned char* packIcon, unsigned int packIconSize,
                                                   const std::wstring& packIconName)
{
    if (gameIconSize != 0 && gameIcon != NULL)
        registerSubstitutionTexture(gameIconName, gameIcon, gameIconSize, false);
    if (packIconSize != 0 && packIcon != NULL)
        registerSubstitutionTexture(packIconName, packIcon, packIconSize, false);
    m_gamesList.AddNewItem(label, id, packIconName, gameIconName);
    SetMatchesAvailable(true);
}

void UIScene_LoadCreateJoinMenu::InitFocusJoinList(int)
{
}

void UIScene_LoadCreateJoinMenu::ClearJoinList()
{
    m_gamesList.clearList();
    SetMatchesAvailable(false);
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

void UIScene_LoadCreateJoinMenu::SetJoinListEnabled(bool)
{
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

void UIScene_LoadCreateJoinMenu::ShowNoGamesLabel(bool)
{
}

int UIScene_LoadCreateJoinMenu::DeleteSaveDataReturned(void* context, bool success)
{
    gConsoleUIController.EnterCallbackIdCriticalSection();
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu != NULL)
    {
        if (success)
            menu->m_pendingAction = ePendingAction_ReloadSaveInfo;
        else
            menu->m_ignoreInput = false;
        menu->updateTooltips();
    }
    gConsoleUIController.LeaveCallbackIdCriticalSection();
    return 0;
}

void UIScene_LoadCreateJoinMenu::DeleteSaveFile(int index)
{
    gStorage->DeleteSaveData(&m_saveDetails->saves[index], &DeleteSaveDataReturned,
                             reinterpret_cast<void*>(GetCallbackUniqueId()));
}

void UIScene_LoadCreateJoinMenu::Initialise()
{
    m_selectedGameIndex = 0;
    m_texturePackWorldCount = 0;
    m_selectedCreateIndex = 0;
    m_selectedSaveIndex = 0;
    m_buttonCount = 0;
    if (!gProfile->IsFullVersion() || m_miniGames)
    {
        AddDefaultButtons();
        m_ignoreInput = false;
        gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
        return;
    }
    if (!gStorage->GetSaveDisabled())
        gStorage->EnoughSpaceForAMinSaveGame();
    m_pendingAction = ePendingAction_GetSaveInfo;
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

int UIScene_LoadCreateJoinMenu::RenameSaveDataReturned(void* context, bool success)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu == NULL)
        return 0;
    if (success)
        menu->m_pendingAction = ePendingAction_GetSaveInfo;
    else
        menu->m_ignoreInput = false;
    menu->updateTooltips();
    return 0;
}

int UIScene_LoadCreateJoinMenu::KeyboardCompleteWorldNameCallback(void* context, bool accepted)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu == NULL)
        return 0;
    if (accepted)
    {
        menu->m_ignoreInput = false;
        wchar_t worldName[128];
        std::memset(worldName, 0, sizeof(worldName));
        g_pInput->GetText(worldName, 127);
        if (worldName[0] == L'\0')
        {
            menu->m_ignoreInput = false;
            menu->updateTooltips();
        }
        else
        {
            menu->m_ignoreInput = true;
            gStorage->RenameSaveData(menu->m_selectedSaveIndex, worldName, &RenameSaveDataReturned, context);
        }
    }
    else
    {
        menu->m_ignoreInput = false;
        menu->updateTooltips();
    }
    return 0;
}

int UIScene_LoadCreateJoinMenu::CopySaveErrorDialogFinishedCallback(void* context, int, C4JStorage::EMessageResult)
{
    gConsoleUIController.EnterCallbackIdCriticalSection();
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu != NULL)
        menu->m_copyInProgress = false;
    gConsoleUIController.LeaveCallbackIdCriticalSection();
    return 0;
}

int UIScene_LoadCreateJoinMenu::CopySaveDataReturned(void* context, bool success, C4JStorage::ESaveGameState state)
{
    gConsoleUIController.EnterCallbackIdCriticalSection();
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu == NULL)
    {
        gConsoleUIController.LeaveCallbackIdCriticalSection();
        return 0;
    }
    if (success)
    {
        menu->m_pendingAction = ePendingAction_ReloadSaveInfo;
        menu->m_copyInProgress = false;
        gConsoleUIController.LeaveCallbackIdCriticalSection();
        return 0;
    }
    if (state == C4JStorage::ESaveGame_CopyFailedNoSpace)
    {
        int options[1] = { IDS_CONFIRM_OK };
        gConsoleUIController.LeaveCallbackIdCriticalSection();
        gConsoleUIController.RequestErrorMessage(IDS_COPYSAVE_FAILED_TITLE, 0x5D2F512D, options, 1,
                                                 gProfile->GetPrimaryPad(), &CopySaveErrorDialogFinishedCallback,
                                                 context, NULL, true);
        return 0;
    }
    menu->m_copyInProgress = false;
    gConsoleUIController.LeaveCallbackIdCriticalSection();
    return 0;
}

bool UIScene_LoadCreateJoinMenu::CopySaveDataProgress(void* context, int percent)
{
    bool keepCopying = false;
    gConsoleUIController.EnterCallbackIdCriticalSection();
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu != NULL)
        keepCopying = !menu->m_copyCancelled;
    gConsoleUIController.LeaveCallbackIdCriticalSection();
    Minecraft::GetInstance()->mProgressRenderer->progressStagePercentage(percent);
    return keepCopying;
}

int UIScene_LoadCreateJoinMenu::CopySaveThreadProc(void* context)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mProgressRenderer->progressStart(IDS_PROGRESS_COPYING_SAVE);
    minecraft->mProgressRenderer->progressStage(-1);

    gConsoleUIController.EnterCallbackIdCriticalSection();
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (menu != NULL)
    {
        menu->m_copyCancelled = false;
        menu->m_copyInProgress = true;
        gConsoleUIController.LeaveCallbackIdCriticalSection();
        gStorage->CopySaveData(&menu->m_saveDetails->saves[menu->m_selectedSaveIndex], &CopySaveDataReturned,
                               &CopySaveDataProgress, context);

        while (true)
        {
            Sleep(COPY_POLL_INTERVAL);
            gConsoleUIController.EnterCallbackIdCriticalSection();
            menu = static_cast<UIScene_LoadCreateJoinMenu*>(
                gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
            if (menu == NULL)
                break;
            bool copying = menu->m_copyInProgress;
            gConsoleUIController.LeaveCallbackIdCriticalSection();
            if (!copying)
                return 0;
        }
    }
    gConsoleUIController.LeaveCallbackIdCriticalSection();
    return 0;
}

int UIScene_LoadCreateJoinMenu::CopySaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (result != C4JStorage::EMessageResult_Accept)
    {
        menu->m_ignoreInput = false;
        return 0;
    }
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadParam = reinterpret_cast<void*>(menu->GetCallbackUniqueId());
    params->m_waitForThreadToDelete = true;
    params->m_threadProc = &CopySaveThreadProc;
    FullscreenProgressState* progressState = new FullscreenProgressState;
    params->m_completionData = progressState;
    progressState->closeMode = 6;
    progressState->playerId = 0;
    gConsoleUIController.NavigateToScene(pad, EUIScene_FullscreenProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
    return 0;
}

int UIScene_LoadCreateJoinMenu::LoadSaveDataForUploadReturned(void* context, bool failed, bool)
{
    static_cast<UIScene_LoadCreateJoinMenu*>(context)->m_uploadState =
        failed ? eUploadState_Failed : eUploadState_SaveLoaded;
    return 0;
}

int UIScene_LoadCreateJoinMenu::CrossSaveUploadFinishedCallback(void* context, int, C4JStorage::EMessageResult)
{
    static_cast<UIScene_LoadCreateJoinMenu*>(context)->m_uploadState = eUploadState_Finished;
    return 0;
}

int UIScene_LoadCreateJoinMenu::UploadNintendoLanSaveThreadProc(void* context)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    menu->m_uploadCancelled = false;
    Minecraft* minecraft = Minecraft::GetInstance();
    ProgressRenderer* progress = minecraft->mProgressRenderer;
    progress->progressStagePercentage(0);
    progress->progressStart(IDS_TITLE_SAVETRANSFER);
    progress->progressStage(IDS_SAVETRANSFER_INITIALISING_NETWORK);

    SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
    SAVE_INFO& save = details->saves[menu->m_selectedSaveIndex];
    menu->m_uploadState = eUploadState_LoadingSave;
    if (gStorage->LoadSaveData(&save, &LoadSaveDataForUploadReturned, menu) == C4JStorage::ESaveGame_Load)
    {
        menu->m_uploadState = eUploadState_LoadingSave;
        progress->progressStagePercentage(0);
    }
    else
    {
        menu->m_uploadState = eUploadState_Failed;
    }

    NQRNetworkManager_Lan_WiiU* lan = gLanNetworkManager;
    arrayWithLength<unsigned char> saveData;
    EUploadState startState = menu->m_uploadState;
    int okOption[1] = { IDS_CONFIRM_OK };
    while (menu->m_uploadState != eUploadState_Finished)
    {
        switch (menu->m_uploadState)
        {
        case eUploadState_SaveLoaded:
            CGameNetworkManager::sInstance.setLanMode(true);
            menu->m_uploadState = eUploadState_StartingLanSession;
            break;
        case eUploadState_StartingLanSession:
            if (menu->m_uploadState != startState)
                progress->progressStage(IDS_SAVETRANSFER_INITIALISING_NETWORK);
            if (CGameNetworkManager::sInstance.isLanModeChangeInProgress())
            {
                if (menu->m_uploadCancelled)
                    menu->m_uploadState = eUploadState_Finished;
                break;
            }
            switch (lan->m_saveTransferState)
            {
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_CreatingSession:
                break;
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_CreateSessionFailed:
                menu->m_uploadState = eUploadState_Failed;
                break;
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_WaitingForReceiver:
                menu->m_uploadState = eUploadState_WaitingForReceiver;
                break;
            default:
            {
                if (!CGameNetworkManager::sInstance.isLanConnected())
                {
                    menu->m_uploadState = eUploadState_Failed;
                    break;
                }
                unsigned int saveSize = gStorage->GetSaveSize();
                saveData.resize(saveSize);
                gStorage->GetSaveData(saveData.data, &saveSize);
                if (save.thumbnail == NULL)
                    break;
                CMinecraftApp::PngTagData tagData;
                gApp.GetImageTextData(save.thumbnail, save.thumbnailSize, &tagData);
                lan->CreateSessionForSaveTransfer(saveData.length, saveData.data, save.thumbnailSize,
                                                  save.thumbnail, tagData.seed, tagData.hostOptions,
                                                  tagData.texturePack, save.title);
                break;
            }
            }
            break;
        case eUploadState_Cancelled:
            menu->m_uploadState = eUploadState_Finished;
            break;
        case eUploadState_WaitingForReceiver:
            if (menu->m_uploadCancelled)
            {
                menu->m_uploadState = eUploadState_Finished;
                break;
            }
            progress->progressStage(0xB9F3A58A);
            if (lan->m_saveTransferState == NQRNetworkManager_Lan_WiiU::eSaveTransfer_Transferring)
                menu->m_uploadState = eUploadState_Transferring;
            break;
        case eUploadState_Transferring:
        {
            if (menu->m_uploadCancelled)
            {
                menu->m_uploadState = eUploadState_Finished;
                break;
            }
            wchar_t text[256];
            switch (lan->m_saveTransferState)
            {
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_Idle:
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_ConnectionLost:
                menu->m_uploadState = eUploadState_Failed;
                break;
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_Transferring:
            {
                int percent = static_cast<int>(lan->m_saveTransferOffset * 100) /
                              static_cast<int>(lan->m_saveTransferSize);
                std::swprintf(text, 256, L10N::GetString(0x0CE6C914), percent);
                progress->progressStage(std::wstring(text));
                progress->progressStagePercentage(percent);
                break;
            }
            case NQRNetworkManager_Lan_WiiU::eSaveTransfer_SendComplete:
                std::swprintf(text, 256, L10N::GetString(0xB7147763), 100);
                progress->progressStage(std::wstring(text));
                progress->progressStagePercentage(100);
                menu->m_uploadState = eUploadState_Complete;
                break;
            default:
                break;
            }
            break;
        }
        case eUploadState_Complete:
            gConsoleUIController.RequestErrorMessage(IDS_TITLE_SAVETRANSFER, 0x4925B633, okOption, 1,
                                                     gProfile->GetPrimaryPad(), &CrossSaveUploadFinishedCallback,
                                                     menu, NULL, true);
            menu->m_uploadState = eUploadState_CompleteShown;
            break;
        case eUploadState_Failed:
            if (menu->m_uploadCancelled)
            {
                menu->m_uploadState = eUploadState_Finished;
                break;
            }
            gConsoleUIController.RequestErrorMessage(IDS_TITLE_SAVETRANSFER, IDS_SAVE_TRANSFER_FAILED, okOption, 1,
                                                     gProfile->GetPrimaryPad(), &CrossSaveUploadFinishedCallback,
                                                     menu, NULL, true);
            menu->m_uploadState = eUploadState_FailedShown;
            break;
        default:
            break;
        }
        Sleep(UPLOAD_POLL_INTERVAL);
    }

    CGameNetworkManager::sInstance.setLanMode(false);
    EnterCriticalSection(&menu->m_lock);
    menu->m_returnToHomeMenu = true;
    LeaveCriticalSection(&menu->m_lock);
    return 0;
}

int UIScene_LoadCreateJoinMenu::CancelSaveUploadCallback(void* context)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    menu->m_uploadCancelled = true;
    gConsoleUIController.SetTooltips(0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    menu->m_ignoreInput = true;
    return 0;
}

void UIScene_LoadCreateJoinMenu::LaunchSaveUpload()
{
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &UploadNintendoLanSaveThreadProc;
    params->m_threadParam = this;
    FullscreenProgressState* progressState = new FullscreenProgressState;
    params->m_cancelText = IDS_TOOLTIPS_CANCEL;
    params->m_cancelFunc = &CancelSaveUploadCallback;
    params->m_cancelFuncParam = this;
    params->m_completionData = progressState;
    progressState->closeMode = 6;
    progressState->playerId = 0;
    gConsoleUIController.NavigateToScene(m_playerId, EUIScene_FullscreenProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}

int UIScene_LoadCreateJoinMenu::SaveTransferDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    UIScene_LoadCreateJoinMenu* menu = static_cast<UIScene_LoadCreateJoinMenu*>(context);
    if (result == C4JStorage::EMessageResult_Accept)
        menu->LaunchSaveUpload();
    menu->m_ignoreInput = false;
    return 0;
}

int UIScene_LoadCreateJoinMenu::handleSaveOptionsDialogReturned(int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_ThirdOption && gApp.HasNoSpaceForSaveQuota())
        result = C4JStorage::EMessageResult_FourthOption;

    switch (result)
    {
    case C4JStorage::EMessageResult_Accept:
        m_ignoreInput = true;
        g_pInput->RequestKeyboard(L10N::GetString(IDS_RENAME_WORLD_TITLE),
                                  m_saveListDetails[m_selectedSaveIndex].m_title, 0, 25,
                                  &KeyboardCompleteWorldNameCallback,
                                  reinterpret_cast<void*>(GetCallbackUniqueId()), C_4JInput::EKeyboardMode_0);
        break;
    case C4JStorage::EMessageResult_Decline:
    {
        int options[2] = { IDS_CONFIRM_CANCEL, IDS_CONFIRM_OK };
        gConsoleUIController.RequestAlertMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, options, 2, pad,
                                                 &DeleteSaveDialogReturned, this, NULL, true);
        break;
    }
    case C4JStorage::EMessageResult_ThirdOption:
    {
        int options[2] = { IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL };
        gConsoleUIController.RequestAlertMessage(IDS_COPYSAVE, IDS_TEXT_COPY_SAVE, options, 2, pad,
                                                 &CopySaveDialogReturned, this, NULL, true);
        break;
    }
    case C4JStorage::EMessageResult_FourthOption:
    {
        if (!gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
            break;
        int options[2] = { IDS_CONFIRM_OK, IDS_CONFIRM_CANCEL };
        SAVE_INFO& save = gStorage->ReturnSavesInfo()->saves[m_selectedSaveIndex];
        unsigned char* thumbnail = NULL;
        unsigned int thumbnailSize = 0;
        if (save.thumbnail != NULL)
        {
            thumbnailSize = save.thumbnailSize;
            thumbnail = save.thumbnail;
        }
        CMinecraftApp::PngTagData tagData;
        if (thumbnail != NULL)
            gApp.GetImageTextData(thumbnail, thumbnailSize, &tagData);
        if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_ExtraData) == 0 ||
            gApp.GetExtraGameData(tagData.extraData, eGameExtraData_SaveVersion) < EXTRA_DATA_SAVE_VERSION)
            gConsoleUIController.RequestAlertMessage(IDS_TITLE_SAVETRANSFER, 0x129BC197, options, 2, pad,
                                                     &SaveTransferDialogReturned, this, NULL, true);
        else
            gConsoleUIController.RequestAlertMessage(IDS_TITLE_SAVETRANSFER, 0x1E0353F0, options, 2, pad,
                                                     &SaveTransferDialogReturned, this, NULL, true);
        break;
    }
    default:
        updateTooltips();
        m_ignoreInput = false;
        break;
    }
    return 0;
}

int UIScene_LoadCreateJoinMenu::handleTexturePackDialogReturned(int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Accept)
    {
        XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
        if (gProfile->IsSignedIn(pad))
        {
            int options[1] = { IDS_CONFIRM_OK };
            if (!gProfile->IsSignedInLive(pad))
                gConsoleUIController.RequestErrorMessage(0x8A21894B, 0xC34C0B77, options, 1, pad, NULL, NULL, NULL,
                                                         true);
            else if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(
                         m_joinSceneData->session->m_gameSessionData.m_texturePackParentId))
                gConsoleUIController.RequestErrorMessage(IDS_DLC_ALREADY_INSTALLED_TITLE,
                                                         IDS_DLC_ALREADY_INSTALLED_TEXT, options, 1, pad, NULL, NULL,
                                                         NULL, true);
        }
    }
    m_ignoreInput = false;
    return 0;
}

int UIScene_LoadCreateJoinMenu::getSceneType()
{
    return EUIScene_LoadCreateJoinMenu;
}

unsigned int UIScene_LoadCreateJoinMenu::getSubSceneType() const
{
    return m_miniGames;
}

int UIScene_LoadCreateJoinMenu::GetPad()
{
    return m_playerId;
}
