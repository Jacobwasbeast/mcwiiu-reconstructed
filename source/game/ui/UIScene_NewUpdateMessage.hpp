#ifndef GAME_UI_UISCENE_NEWUPDATEMESSAGE_HPP
#define GAME_UI_UISCENE_NEWUPDATEMESSAGE_HPP

#include <string>

#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_DynamicLabel.hpp"
#include "game/ui/UIScene.hpp"

class UILayer;

class UIScene_NewUpdateMessage : public UIScene
{
public:
    UIScene_NewUpdateMessage(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_NewUpdateMessage();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handlePress(int first, int second);
    virtual int getSceneType();
    virtual bool hasFocus(int sceneType);
    virtual unsigned int getDefaultGtcButtons();

private:
    bool m_unknownC8;
    unsigned char m_reservedC9[3];
    UIControl_Button m_acceptButton;
    UIControl_DynamicLabel m_eulaText;
};

#endif
