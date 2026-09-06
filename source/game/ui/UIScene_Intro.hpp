#ifndef GAME_UI_UISCENE_INTRO_HPP
#define GAME_UI_UISCENE_INTRO_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Touch.hpp"

class UILayer;

class UIScene_Intro : public UIScene
{
public:
    UIScene_Intro(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_Intro();

    void setIntroPlatform();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleAnimationEnd();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleReload();
    virtual void handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat,
                                  bool released, bool pressed);
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual bool hasFocus(int sceneType);

private:
    bool m_inputHandled;
    bool m_introFocus;
    UIControl_Touch m_touchControl;
};

#endif
