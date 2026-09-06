#ifndef GAME_UI_UISCENE_REINSTALLMENU_HPP
#define GAME_UI_UISCENE_REINSTALLMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"

class UILayer;

class UIScene_ReinstallMenu : public UIScene
{
public:
    UIScene_ReinstallMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_ReinstallMenu();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handlePress(int first, int second);
    virtual int getSceneType();

private:
    UIControl_Button m_button1;
    UIControl_Button m_button2;
    UIControl_Button m_button3;
    UIControl_Button m_button4;
    UIControl_Button m_button5;
    UIControl_Button m_button6;
};

#endif
