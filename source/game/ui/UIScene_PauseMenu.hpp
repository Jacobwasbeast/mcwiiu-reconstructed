#ifndef GAME_UI_UISCENE_PAUSEMENU_HPP
#define GAME_UI_UISCENE_PAUSEMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UIScene_PauseMenu : public UIScene
{
public:
    UIScene_PauseMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_PauseMenu();
    static unsigned int SaveWorldThreadProc(void* context);
    static unsigned int NetworkErrorHandledDuringInvite(void* context, int userIndex, C4JStorage::EMessageResult result);
    static void _ExitWorld(void* context);
    static unsigned int ExitWorldThreadProc(void* context);
    void OpenExitGameDialog(int player, void* callback, void* saveCallback, int action);
    static unsigned int WarningTrialTexturePackReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int ExitGameDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int ExitGameAndSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int ExitGameDeclineSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int ExitGameSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int SaveGameDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int EnableAutosaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static unsigned int DisableAutosaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    static unsigned int UnlockFullSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    void PerformActionSaveGame();
    static unsigned int SaveGame_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    void Init();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handlePreReload();
    virtual void handleReload();
    virtual std::wstring getMoviePath();
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled);
    virtual void handlePress(int first, int second);
    void ShowScene(bool show);
    void SetIgnoreInput(bool ignore);
    static unsigned int BuyTexturePack_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    unsigned int IsBanEnabled();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    static unsigned int MustSignInReturnedNintendo(void* context, int button, C4JStorage::EMessageResult result);
private:
    UIControl_MultiList m_buttonList;
    bool m_ignoreInput;
};

#endif
