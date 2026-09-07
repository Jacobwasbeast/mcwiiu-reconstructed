#ifndef GAME_UI_UISCENE_CREATEWORLDMENU_HPP
#define GAME_UI_UISCENE_CREATEWORLDMENU_HPP

#include <string>

#include "game/core/IUIScene_StartGame.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Slider.hpp"
#include "game/ui/UIControl_TextInput.hpp"

class UILayer;

class UIScene_CreateWorldMenu : public IUIScene_StartGame
{
public:
    UIScene_CreateWorldMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_CreateWorldMenu();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleDestroy();
    virtual void tick();
    virtual unsigned int GetMainPanel();
    virtual std::wstring getMoviePath();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    static int KeyboardCompleteWorldNameCallback(void* data, bool accepted);
    virtual void handlePress(int first, int second);
    virtual void handleCheckboxToggled(int id, bool checked);
    void checkStateAndStartGame();
    void SetEnablePlayGameButton(bool enable);
    virtual int getSceneType();

private:
    UIControl_Label m_worldNameLabel;
    UIControl_Button m_moreOptionsButton;
    UIControl_Button m_playButton;
    UIControl_TextInput m_worldNameInput;
    UIControl_BitmapIcon m_worldIcon;
    UIControl_Slider m_gameModeSlider;
    UIControl_CheckBox m_onlineCheckbox;
    std::wstring m_worldName;
    bool m_keyboardActive;
    void* m_callbackData;
};

#endif
