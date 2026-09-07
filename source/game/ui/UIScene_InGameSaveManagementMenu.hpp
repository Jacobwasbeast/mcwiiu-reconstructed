#ifndef GAME_UI_UISCENE_INGAMESAVEMANAGEMENTMENU_HPP
#define GAME_UI_UISCENE_INGAMESAVEMANAGEMENTMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SpaceIndicatorBar.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UIScene_InGameSaveManagementMenu : public UIScene
{
public:
    UIScene_InGameSaveManagementMenu(int sceneType, void* sceneData, UILayer* layer);
    virtual ~UIScene_InGameSaveManagementMenu();

    void GetSaveInfo();
    void Initialise();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void handleReload();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleInitFocus(int focus, int childFocus);
    virtual bool hasFocus(int sceneType);
    static unsigned int LoadSaveDataThumbnailReturned(void* context, unsigned char* data,
                                                      unsigned int size);
    virtual void tick();
    virtual std::wstring getMoviePath();
    static unsigned int DeleteSaveDataReturned(void* context, bool success);
    static unsigned int DeleteSaveDialogReturned(void* context, int button,
                                                  C4JStorage::EMessageResult result);
    virtual void handlePress(int first, int second);
    virtual int getSceneType();
    virtual bool hidesLowerScenes();

private:
    unsigned char* m_saveDetails;
    int m_saveCount;
    UIControl_SaveList m_saveList;
    UIControl_Label m_saveListTitle;
    UIControl m_saveTimer;
    UIControl_SpaceIndicatorBar m_saveSizeBar;
    int m_action;
    bool m_deletePending;
    bool m_saveInfoPending;
    bool m_thumbnailPending;
    bool m_thumbnailReturned;
    int m_currentSave;
    void* m_saveInfo;
    bool m_saveInfoReady;
    bool m_closeRequested;
    int m_saveOperation;
    int m_focusSelection;
    bool m_focusChanged;
};

#endif
