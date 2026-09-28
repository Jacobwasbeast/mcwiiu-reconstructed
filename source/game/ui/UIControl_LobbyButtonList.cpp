#include "game/ui/UIControl_LobbyButtonList.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_LobbyButtonList::~UIControl_LobbyButtonList()
{
}

void UIControl_LobbyButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_LobbyButtonList;
    UIControl_ButtonList::setupControl(scene, node, name);
}

void UIControl_LobbyButtonList::addItem(const std::wstring& label, bool host, int playerIcon, int voipIcon)
{
    m_label = label;
    m_host = host;
    m_playerIcon = playerIcon;
    m_voipIcon = voipIcon;
    m_item += 1;
}

void UIControl_LobbyButtonList::setPlayerIcon(int item, int icon)
{
    if (item == m_item)
    {
        m_playerIcon = icon;
    }
}

void UIControl_LobbyButtonList::setVOIPIcon(int item, int icon)
{
    if (item == m_item)
    {
        m_voipIcon = icon;
    }
}

void UIControl_LobbyButtonList::showTick(int item, bool show)
{
    if (item == m_item)
    {
        m_tickVisible = show;
    }
}
