#include "game/ui/UIControl_ButtonList.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List_ListButton.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/FJ_List_MenuButton.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_ButtonList::UIControl_ButtonList()
{
}

UIControl_ButtonList::~UIControl_ButtonList()
{
}

bool UIControl_ButtonList::setupControl(UIScene* scene, fuiRenderNode* node,
                                        const std::string& name)
{
    m_controlType = 0x16;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_ButtonList::addItem(const std::wstring& label, int index)
{
    if (m_scene->hasMovie()) {
        FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
        FJ_List_ListButton* listButton = node->asFJ_List_ListButton();
        if (listButton != NULL) {
            listButton->addNewItem(label, index);
            addElement(static_cast<UIControl::eUIControlType>(1), -1);
            return;
        }

        FJ_List_MenuButton* menuButton = node->asFJ_List_MenuButton();
        if (menuButton != NULL) {
            menuButton->addNewItem(std::wstring(label), index);
            addElement(static_cast<UIControl::eUIControlType>(1), -1);
            return;
        }

        FJ_List_ListButtonPlayerList* playerList = node->asFJ_List_ListButtonPlayerList();
        if (playerList != NULL) {
            playerList->addNewItem(label, index, 1, 1);
            addElement(static_cast<UIControl::eUIControlType>(1), -1);
            return;
        }

        FJ_List_ListButtonScoreBoard* scoreBoard = node->asFJ_List_ListButtonScoreBoard();
        if (scoreBoard != NULL)
            scoreBoard->addNewItem(label, index, 1, 1);
    }

    addElement(static_cast<UIControl::eUIControlType>(1), -1);
}

void UIControl_ButtonList::addItem(const std::wstring& label)
{
    addItem(label, static_cast<int>(getItemCount()));
}

void UIControl_ButtonList::setButtonLabel(int item, const std::wstring& label)
{
    UIString value(label);
    SetItemLabel(item, value, true);
}
