#include "game/ui/UIScene_InGameSaveManagementMenu.hpp"

#include <cstring>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "platform/wiiu/CStorage.hpp"

std::wstring convUtf8ToWstring(const char* value);
extern DLCManager g_DLCManager;

namespace
{
    struct SaveListRecord
    {
        unsigned int thumbnail;
        unsigned int thumbnailSize;
        wchar_t title[64];
        char filename[32];
    };

    static SaveListRecord* getSaveListRecord(unsigned char* details, int index)
    {
        return (SaveListRecord*)(details + index * 300);
    }
}

UIScene_InGameSaveManagementMenu::UIScene_InGameSaveManagementMenu(int sceneType, void* sceneData,
                                                                   UILayer* layer)
    : UIScene(sceneType, layer), m_saveDetails(NULL), m_saveCount(0), m_saveList(),
      m_saveListTitle(), m_saveTimer(), m_saveSizeBar(), m_action(0), m_deletePending(false),
      m_saveInfoPending(false), m_thumbnailPending(false), m_thumbnailReturned(false),
      m_currentSave(0), m_saveInfo(NULL), m_saveInfoReady(false), m_closeRequested(false),
      m_saveOperation(0), m_focusSelection(0), m_focusChanged(false)
{
    initialiseMovie();
    m_saveList.init(0);
    m_saveListTitle.init(UIString(L10N::GetString(0xfec9f75f)));
    m_saveTimer.setVisible(true);
    m_saveSizeBar.init(UIString(L"\\"), 1, 0, 0);
    m_action = 0;
    m_saveInfo = NULL;
    m_saveInfoReady = false;
    m_currentSave = 0;
    m_saveCount = 0;
    m_saveDetails = NULL;
    m_saveInfoPending = false;
    m_closeRequested = false;
    m_saveOperation = 0;
    m_thumbnailPending = false;
    m_deletePending = false;
    m_thumbnailReturned = false;
    m_focusChanged = false;
    gApp.EnterSaveNotificationSection();
    if (gApp.StartInstallDLCProcess(sceneType) == 0 && gApp.DLCInstallPending() == 0)
    {
        Initialise();
        if (!m_deletePending)
            g_DLCManager.checkForCorruptDLCAndAlert(true);
    }
    m_layer->addComponent(sceneType, (EUIScene)0x48, sceneData);
}

UIScene_InGameSaveManagementMenu::~UIScene_InGameSaveManagementMenu()
{
    if (m_layer != NULL)
        m_layer->removeComponent((EUIScene)0x48);
    if (m_saveDetails != NULL)
    {
        for (int i = 0; i < m_saveCount; ++i)
            delete[] (unsigned char*)getSaveListRecord(m_saveDetails, i)->thumbnail;
        delete[] m_saveDetails;
    }
    gApp.LeaveSaveNotificationSection();
}

void UIScene_InGameSaveManagementMenu::GetSaveInfo()
{
    m_saveInfoReady = false;
    m_saveList.clearList();
    m_saveInfoPending = false;
    m_saveOperation = 0;
    m_saveTimer.setVisible(true);
    m_saveInfo = (void*)(uintptr_t)gStorage->ReturnSavesInfo();
    if (m_saveInfo == NULL)
        gStorage->GetSavesInfo(m_sceneType, NULL, this, "save");
}

void UIScene_InGameSaveManagementMenu::Initialise()
{
    m_focusSelection = 0;
    if (gStorage->GetSaveState() != 0)
    {
        if (gStorage->ReturnSavesInfo() == 0)
            gStorage->ClearSavesInfo();
        GetSaveInfo();
    }
    m_deletePending = false;
}

unsigned int UIScene_InGameSaveManagementMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_saveList.setupControl(this, root, std::string("SavesList"));
    m_controls.push_back(&m_saveList);
    m_saveListTitle.setupControl(this, root, std::string("SavesListTitle"));
    m_controls.push_back(&m_saveListTitle);
    m_saveTimer.setupControl(this, root, std::string("SavesTimer"));
    m_controls.push_back(&m_saveTimer);
    m_saveSizeBar.setupControl(this, root, std::string("SaveSizeBar"));
    m_controls.push_back(&m_saveSizeBar);
    return 1;
}

void UIScene_InGameSaveManagementMenu::updateTooltips()
{
    int tooltip = (m_saveInfoReady && m_saveCount > 0) ? (int)0x0e3265e5 : -1;
    gConsoleUIController.SetTooltips((unsigned int)m_sceneType, tooltip, (int)0xaed18de8,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, false, m_layer->IsFullscreenGroup());
}

void UIScene_InGameSaveManagementMenu::handleReload()
{
    m_action = 0;
    m_currentSave = 0;
    m_saveInfoReady = false;
    m_saveOperation = 0;
    m_thumbnailPending = false;
    m_deletePending = false;
}

void UIScene_InGameSaveManagementMenu::handleGainFocus(bool hadFocus)
{
    updateTooltips();
    if (hadFocus)
        m_deletePending = false;
}

void UIScene_InGameSaveManagementMenu::handleInput(int playerId, int action, bool repeat,
                                                   bool released, bool pressed, bool& handled)
{
    if (m_deletePending || !m_saveInfoReady)
        return;
    gConsoleUIController.AnimateKeyPress((unsigned int)m_sceneType, action, repeat, released,
                                         pressed);
    if (action < 4 || (action > 5 && action < 8))
        return;
    if (action > 9 && action != 0x14)
    {
        if (action != 0x15 || !released)
            return;
        navigateBack();
        handled = true;
        return;
    }
    sendInputToMovie(action, repeat, released, pressed);
    handled = true;
}

void UIScene_InGameSaveManagementMenu::handleFocusChange(int focus, int childFocus)
{
    m_focusSelection = childFocus;
    if (m_saveInfoReady)
        m_focusChanged = true;
    UIScene::handleFocusChange(focus, childFocus);
}

void UIScene_InGameSaveManagementMenu::handleInitFocus(int, int)
{
}

bool UIScene_InGameSaveManagementMenu::hasFocus(int sceneType)
{
    return m_hasFocus && (sceneType == m_sceneType || m_sceneType == 0xff);
}

unsigned int UIScene_InGameSaveManagementMenu::LoadSaveDataThumbnailReturned(void* context,
                                                                              unsigned char* data,
                                                                              unsigned int size)
{
    UIScene_InGameSaveManagementMenu* menu = (UIScene_InGameSaveManagementMenu*)context;
    SaveListRecord* record = getSaveListRecord(menu->m_saveDetails, menu->m_currentSave);
    if (data == NULL || size == 0)
    {
        menu->m_thumbnailReturned = true;
        record->thumbnail = 0;
        record->thumbnailSize = 0;
    }
    else
    {
        record->thumbnail = (unsigned int)(uintptr_t)new unsigned char[size];
        std::memcpy((void*)(uintptr_t)record->thumbnail, data, size);
        menu->m_thumbnailReturned = true;
        record->thumbnailSize = size;
    }
    return 0;
}

void UIScene_InGameSaveManagementMenu::tick()
{
    UIScene::tick();
    if (m_closeRequested && !m_thumbnailPending)
        navigateBack();
    if (m_focusChanged && !m_thumbnailPending)
    {
        m_saveSizeBar.selectSave(m_focusSelection);
        m_focusChanged = false;
    }
    if (gStorage->GetSaveState() != 0)
    {
        if (!m_saveInfoReady)
        {
            SAVE_DETAILS* details = (SAVE_DETAILS*)(uintptr_t)gStorage->ReturnSavesInfo();
            if (details != NULL)
            {
                m_saveInfoReady = true;
                m_saveSizeBar.reset();
                m_saveCount = details->count;
                m_saveDetails = (unsigned char*)new unsigned char[m_saveCount * 300];
                for (int i = 0; i < m_saveCount; ++i)
                {
                    SaveListRecord* record = getSaveListRecord(m_saveDetails, i);
                    record->thumbnail = 0;
                    record->thumbnailSize = 0;
                    std::memcpy(record->title, details->saves[i].title, 256);
                    std::memcpy(record->filename, details->saves[i].filename, 32);
                    m_saveSizeBar.addSave(details->saves[i].nandSize);
                    m_saveList.addItem(std::wstring(record->title), std::wstring(L"Save"));
                }
                m_saveTimer.setVisible(false);
            }
        }
        if (m_saveInfoReady && !m_thumbnailPending && !m_thumbnailReturned &&
            m_currentSave < (int)m_saveList.getItemCount())
        {
            m_thumbnailPending = true;
            if (gStorage->LoadSaveDataThumbnail(&((SAVE_DETAILS*)m_saveInfo)->saves[m_currentSave],
                                                (int (*)(void*, unsigned char*, unsigned int))LoadSaveDataThumbnailReturned,
                                                this) != 0x15)
                m_thumbnailReturned = true;
        }
        if (m_thumbnailReturned)
        {
            m_thumbnailReturned = false;
            m_thumbnailPending = false;
            SaveListRecord* record = getSaveListRecord(m_saveDetails, m_currentSave);
            std::wstring texture = convUtf8ToWstring(record->filename);
            if (record->thumbnail != 0)
                registerSubstitutionTexture(texture, (unsigned char*)(uintptr_t)record->thumbnail,
                                             record->thumbnailSize, false);
            m_saveList.setTextureName(m_currentSave, texture);
            ++m_currentSave;
        }
    }
    if (m_action == 2)
    {
        handleReload();
        m_saveList.clearList();
        m_action = 0;
    }
}

std::wstring UIScene_InGameSaveManagementMenu::getMoviePath()
{
    return std::wstring(L"SaveMenu");
}

unsigned int UIScene_InGameSaveManagementMenu::DeleteSaveDataReturned(void* context, bool success)
{
    UIScene_InGameSaveManagementMenu* menu = (UIScene_InGameSaveManagementMenu*)context;
    if (!success)
        menu->m_deletePending = false;
    else
        menu->m_action = 2;
    menu->updateTooltips();
    return 0;
}

unsigned int UIScene_InGameSaveManagementMenu::DeleteSaveDialogReturned(void* context, int,
                                                                        C4JStorage::EMessageResult result)
{
    UIScene_InGameSaveManagementMenu* menu = (UIScene_InGameSaveManagementMenu*)context;
    if (result == (C4JStorage::EMessageResult)5 &&
        (gApp.DebugSettingsOn() == 0 || gApp.GetLoadSavesFromFolderEnabled() == 0))
    {
        SAVE_DETAILS* details = (SAVE_DETAILS*)(uintptr_t)menu->m_saveInfo;
        gStorage->DeleteSaveData(&details->saves[menu->m_focusSelection],
                                 (int (*)(void*, bool))DeleteSaveDataReturned,
                                 menu);
        menu->m_saveTimer.setVisible(true);
    }
    else
        menu->m_deletePending = false;
    return 0;
}

void UIScene_InGameSaveManagementMenu::handlePress(int first, int)
{
    if (first == 0)
    {
        m_deletePending = true;
        int parameters[2] = { (int)0x4fcbc7e7, (int)0xb9a1bf4d };
        gConsoleUIController.RequestErrorMessage(0x0e3265e5, (int)0xc2cd9559, parameters, 2,
                                                  (unsigned int)m_sceneType,
                                                  (int (*)(void*, int, C4JStorage::EMessageResult))DeleteSaveDialogReturned,
                                                  this, NULL, true);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    }
}

int UIScene_InGameSaveManagementMenu::getSceneType()
{
    return 0x1b;
}

bool UIScene_InGameSaveManagementMenu::hidesLowerScenes()
{
    return true;
}
