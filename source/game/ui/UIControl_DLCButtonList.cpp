#include "game/ui/UIControl_DLCButtonList.hpp"
#include "game/ui/FJ_List_ListButtonDLC.hpp"
#include "game/ui/fuiRenderNode.hpp"

std::wstring convStringToWstring(const std::string& value);

UIControl_DLCButtonList::~UIControl_DLCButtonList()
{
}

bool UIControl_DLCButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 0x16;
    return UIControl_List::setupControl(scene, node, name);
}

void UIControl_DLCButtonList::addItem(const std::string& label, bool enabled, int index)
{
    std::wstring wideLabel = convStringToWstring(label);
    FJ_List_ListButtonDLC* list = static_cast<FJ_List_ListButtonDLC*>(m_fuiRenderNode->getFuiNode());
    list->addNewItem(wideLabel, index, enabled);
    addElement(static_cast<UIControl::eUIControlType>(1), index);
}

void UIControl_DLCButtonList::showTick(int item, bool show)
{
    FJ_List_ListButtonDLC* list = static_cast<FJ_List_ListButtonDLC*>(m_fuiRenderNode->getFuiNode());
    list->ShowTick(item, show);
}
