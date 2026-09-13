#ifndef GAME_UI_UICONTROL_LOBBYBUTTONLIST_HPP
#define GAME_UI_UICONTROL_LOBBYBUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_ButtonList.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_LobbyButtonList : public UIControl_ButtonList
{
public:
    virtual ~UIControl_LobbyButtonList();
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void addItem(const std::wstring& label, bool host, int playerIcon, int voipIcon);
    void setPlayerIcon(int item, int icon);
    void setVOIPIcon(int item, int icon);
    void showTick(int item, bool show);

private:
    std::wstring m_label;
    bool m_host;
    int m_playerIcon;
    int m_voipIcon;
    int m_item;
    bool m_tickVisible;
};

#endif
