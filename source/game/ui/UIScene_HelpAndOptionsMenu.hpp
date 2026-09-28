#ifndef GAME_UI_UISCENE_HELPANDOPTIONSMENU_HPP
#define GAME_UI_UISCENE_HELPANDOPTIONSMENU_HPP

#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIScene.hpp"

class UIScene_HelpAndOptionsMenu : public UIScene
{
public:
    UIScene_HelpAndOptionsMenu(int sceneType, void* sceneData, UILayer* layer);
    virtual ~UIScene_HelpAndOptionsMenu();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();
private:
    UIControl_Button m_button1;
    UIControl_Button m_button2;
    UIControl_Button m_button3;
    UIControl_Button m_button4;
    UIControl_Button m_button5;
    UIControl_Button m_button6;
    UIControl_Button m_button7;
    UIControl_Button m_buttonUpsell;
    bool m_unknownFlag;
};

#endif
