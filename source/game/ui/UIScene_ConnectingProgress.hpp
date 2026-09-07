#ifndef GAME_UI_UISCENE_CONNECTINGPROGRESS_HPP
#define GAME_UI_UISCENE_CONNECTINGPROGRESS_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Progress.hpp"

class UIScene_ConnectingProgress : public UIScene
{
public:
    UIScene_ConnectingProgress(int type, void* data, UILayer* layer);
    virtual ~UIScene_ConnectingProgress();

    void SetLoadingScreenType(int type);
    void setMiniGameLogoTexture();
    void ShowConfirmationButton(bool show);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleLoseFocus();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleReload();
    virtual void handlePress(int first, int second);
    void tryRemovePlayer();
    void removeLocalPlayer();

private:
    bool m_timerEnabled;
    int m_timerDuration;
    bool m_miniGame;
    bool m_removePlayer;
    bool m_confirmationVisible;
    bool m_cancelled;
    void (*m_removePlayerCallback)(int);
    int m_removePlayerCallbackData;
    int m_miniGameId;
    bool m_miniGameLoaded;
    UIControl_Progress m_progressControl;
    UIControl_Label m_titleLabel;
    UIControl_Label m_confirmationLabel;
    UIControl_Button m_confirmationButton;
    UIControl m_timerTitleControl;
    UIControl m_timerControl;
    UIControl_BitmapIcon m_logoControl;
};

#endif
