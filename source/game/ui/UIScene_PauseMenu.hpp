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
    static int NetworkErrorHandledDuringInvite(void* context, int userIndex, C4JStorage::EMessageResult result);
    static void _ExitWorld(void* context);
    static unsigned int ExitWorldThreadProc(void* context);
    enum EOpenExitGameDialog
    {
        EOpenExitGameDialog_0 = 0,
        EOpenExitGameDialog_1 = 1
    };
    static void OpenExitGameDialog(
        int player, void* callbackData,
        int (*callback)(void*, int, C4JStorage::EMessageResult),
        int (*saveCallback)(void*, int, C4JStorage::EMessageResult),
        EOpenExitGameDialog action);
    static int WarningTrialTexturePackReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int ExitGameDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int ExitGameAndSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int ExitGameDeclineSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int ExitGameSaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int SaveGameDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int EnableAutosaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    static int DisableAutosaveDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    static int UnlockFullSaveReturned(void* context, int button, C4JStorage::EMessageResult result);
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
    bool m_texturePackWarningPending;
    UIControl_MultiList m_buttonList;
    bool m_ignoreInput;
};

#endif
