#include "game/ui/UIControl_PlayerButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

void UIControl_ButtonList_setupControl(void* control, UIScene* scene, fuiRenderNode* node,
                                       const std::string& name);

UIControl_PlayerButtonList::PendingAddItem::PendingAddItem(
    UIControl_PlayerButtonList* list, int type, std::wstring label, int playerIcon, int voipIcon)
    : UIControl_List::GenericPendingUpdate(type), m_list(list), m_label(label),
      m_playerIcon(playerIcon), m_voipIcon(voipIcon)
{
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
    if (m_scene->hasMovie() == false)
        return;

    m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonPlayerList()->SetVOIPIcon(item, icon + 1);
}

void UIControl_PlayerButtonList::addItem(const std::wstring& label, int playerIcon,
                                        int voipIcon, bool immediate)
{
    if (immediate)
    {
        if (m_scene->hasMovie())
        {
            m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonPlayerList()->addNewItem(
                label, m_itemIndex, playerIcon + 1, voipIcon + 1);
        }
        addElement(static_cast<UIControl::eUIControlType>(1), -1);
        return;
    }

    m_pendingUpdates.push_back(new PendingAddItem(this, 0, label, playerIcon, voipIcon));
}

void UIControl_PlayerButtonList::setupControl(UIScene* scene, fuiRenderNode* node,
                                              const std::string& name)
{
    m_controlType = 0x1d;
    UIControl_ButtonList_setupControl(this, scene, node, name);
}

void UIControl_PlayerButtonList::setPlayerIcon(int item, int icon)
{
    if (m_scene->hasMovie() == false)
        return;

    m_fuiRenderNode->getFuiNode()->asFJ_List_ListButtonPlayerList()->SetPlayerIcon(item, icon + 1);
}
