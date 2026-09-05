#include "game/ui/UIControl_PlayerButtonList.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/fuiRenderNode.hpp"

void UIControl_ButtonList_setupControl(void* control, UIScene* scene, fuiRenderNode* node, const std::string& name);

UIControl_PlayerButtonList::PendingAddItem::PendingAddItem(UIControl_PlayerButtonList* list, int type, std::wstring label, int playerIcon, int voipIcon)
    : UIControl_List::GenericPendingUpdate(type), m_list(list), m_label(), m_playerIcon(playerIcon), m_voipIcon(voipIcon)
{
    m_label = label;
}

UIControl_PlayerButtonList::PendingAddItem::~PendingAddItem()
{
}

unsigned int UIControl_PlayerButtonList::PendingAddItem::OnTick()
{
    m_list->addItem(m_label, m_playerIcon, m_voipIcon, true);
    return 1;
}

UIControl_PlayerButtonList::~UIControl_PlayerButtonList()
{
}

void UIControl_PlayerButtonList::setVOIPIcon(int item, int icon)
{
    if (m_scene->m_ready == 0.0f)
    {
        return;
    }

    static_cast<FJ_List_ListButtonPlayerList*>(m_fuiRenderNode->m_fuiNode)->SetVOIPIcon(item, icon + 1);
}

void UIControl_PlayerButtonList::addItem(const std::wstring& label, int playerIcon, int voipIcon, bool immediate)
{
    if (immediate)
    {
        if (m_scene->m_ready != 0.0f)
        {
            static_cast<FJ_List_ListButtonPlayerList*>(m_fuiRenderNode->m_fuiNode)->addNewItem(label, m_itemIndex, playerIcon + 1, voipIcon + 1);
        }
        addElement(static_cast<UIControl::eUIControlType>(1), -1);
        return;
    }

    PendingAddItem* pending = new PendingAddItem(this, 0, label, playerIcon, voipIcon);
    m_pendingUpdates.push_back(pending);
}

void UIControl_PlayerButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 0x1d;
    UIControl_ButtonList_setupControl(this, scene, node, name);
}

void UIControl_PlayerButtonList::setPlayerIcon(int item, int icon)
{
    if (m_scene->m_ready == 0.0f)
    {
        return;
    }

    static_cast<FJ_List_ListButtonPlayerList*>(m_fuiRenderNode->m_fuiNode)->SetPlayerIcon(item, icon + 1);
}
