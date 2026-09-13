#ifndef GAME_UI_UISCENE_LOADCREATEJOINMENU_HPP
#define GAME_UI_UISCENE_LOADCREATEJOINMENU_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LeftIconButtonList.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UIControl_SpaceIndicatorBar.hpp"
#include "game/ui/UIControl_Touch.hpp"

class UILayer;

class UIScene_LoadCreateJoinMenu : public IUIScene_LoadCreateJoin, public UIScene
{
public:
    UIScene_LoadCreateJoinMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_LoadCreateJoinMenu();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    std::wstring getThumbnailName(int index);
    static int LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size);
    void requestNextThumbnail();
    void cleanUpThumbNails(bool all);
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleLoseFocus();
    virtual UIControl* GetMainPanel();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleDestroy();
    virtual void handleReload();
    virtual void handlePress(int first, int second);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleInitFocus(int focus, int childFocus);
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    virtual void HandleSaveDeviceRemoved(int deviceIndex);
    virtual void handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat,
                                  bool released, bool pressed);
    void SelectTab(int tab);
    void SetMatchesAvailable(bool available);
    void SetHideLoadTab();
    bool GetHideLoadTab();
    void AddLoadListButton(const std::wstring& label, int id);
    void SetLoadListIcon(int index, unsigned char* data, unsigned int size, const std::wstring& name);
    void ScrollLoadListToSelection(unsigned int selection);
    void ClearLoadList();
    unsigned int GetLoadListSize();
    unsigned int GetLoadListSelection();
    void SetLoadListEnabled(bool enabled);
    void AddCreateListButton(const std::wstring& label, int id);
    bool DoesCreateListHaveFocus();
    void SetCreateListIcon(int index, unsigned char* data, unsigned int size, const std::wstring& name);
    void ScrollCreateListToSelection(unsigned int selection);
    void ClearCreateList();
    void SetCreateListEnabled(bool enabled);
    bool AddJoinListButton(const std::wstring& label, int id, unsigned char* data, unsigned int size,
                           const std::wstring& textureName, unsigned char* data1, unsigned int size1,
                           const std::wstring& textureName1);
    void InitFocusJoinList(int focus);
    bool ClearJoinList();
    unsigned int GetJoinListSize();
    unsigned int GetJoinListSelection();
    bool IsJoinListEnabled();
    void SetJoinListEnabled(bool enabled);
    void SetGamesListSelection(unsigned int selection);
    void ShowSaveListTimer(bool show);
    void ShowJoinListTimer(bool show);
    void ShowNoGamesLabel(bool show);
    static uint32_t DeleteSaveDataReturned(void* context, bool success);
    void DeleteSaveFile(int index);
    virtual void Initialise();
    bool DoesLoadListHaveFocus();
    bool DoesMashUpWorldHaveFocus();
    bool DoesGamesListHaveFocus();
    static uint32_t RenameSaveDataReturned(void* context, bool success);
    static uint32_t KeyboardCompleteWorldNameCallback(void* context, bool complete);
    static uint32_t CopySaveErrorDialogFinishedCallback(void* context, int userIndex,
                                                         C4JStorage::EMessageResult result);
    static uint32_t CopySaveDataReturned(void* context, bool success, C4JStorage::ESaveGameState state);
    static bool CopySaveDataProgress(void* context, int progress);
    static uint32_t CopySaveThreadProc(void* context);
    static uint32_t CopySaveDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    static uint32_t LoadSaveDataForUploadReturned(void* context, bool success, bool complete);
    static uint32_t CrossSaveUploadFinishedCallback(void* context, int userIndex,
                                                     C4JStorage::EMessageResult result);
    static uint32_t UploadNintendoLanSaveThreadProc(void* context);
    static void CancelSaveUploadCallback(void* context);
    void LaunchSaveUpload();
    static void SaveTransferDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    void handleSaveOptionsDialogReturned(int userIndex, C4JStorage::EMessageResult result);
    void handleTexturePackDialogReturned(int userIndex, C4JStorage::EMessageResult result);
    virtual int getSceneType();
    virtual unsigned int getSubSceneType() const;
    virtual int GetPad();
    static void LoadSaveCallback(void* context, bool success);

private:
    UIControl_Touch m_loadTouch;
    UIControl_Touch m_createTouch;
    UIControl_Touch m_joinTouch;
    UIControl m_loadTab;
    UIControl m_createTab;
    UIControl m_joinTab;
    UIControl_SaveList m_loadList;
    UIControl_SaveList m_createList;
    UIControl_LeftIconButtonList m_gamesList;
    UIControl_Label m_loadTitle;
    UIControl_Label m_createTitle;
    UIControl_Label m_joinTitle;
    UIControl m_loadTimer;
    UIControl m_createTimer;
    UIControl m_joinTimer;
    UIControl_SpaceIndicatorBar m_saveSizeBar;
    int m_selectedLoad;
    int m_selectedCreate;
    int m_selectedJoin;
    int m_player;
    int m_activeTab;
    unsigned char m_matchesAvailable;
    unsigned char m_hideLoadTab;
    unsigned char m_loadThumbnailPending;
    unsigned char m_createThumbnailPending;
    unsigned char m_joinThumbnailPending;
    unsigned char m_saveOperationPending;
    unsigned char m_saveDeviceRemoved;
    unsigned char m_initialised;
    unsigned char m_focusChanged;
    unsigned int m_thumbnailIndex;
    unsigned int m_callbackId;
    int m_saveDetailsCount;
};

#endif
