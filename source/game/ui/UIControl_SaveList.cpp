#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

void UIControl_ButtonList_setupControl(void* control, UIScene* scene, fuiRenderNode* node, const std::string& name);
void UIControl_List_destroy(void* control, unsigned int deleting);
void UIControl_List_addElement(void* control, int type, int index);
void FJ_List_ListButtonIconLeft_addNewItem(void* list, const std::wstring& label, int index, const std::wstring& textureName, const std::wstring& iconName);
void FJ_List_ListButtonIconLeft_setTextureName(void* list, int item, const std::wstring& textureName);

UIControl_SaveList::~UIControl_SaveList()
{
    UIControl_List_destroy(this, 0);
}

void UIControl_SaveList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 14;
    UIControl_ButtonList_setupControl(this, scene, node, name);
}

void UIControl_SaveList::addItem(const std::wstring& label, const std::wstring& textureName, int icon)
{
    std::wstring iconName(L"buckets_");
    FJ_List_ListButtonIconLeft_addNewItem(m_listRenderNode, label, m_itemIndex, textureName, iconName);
}

void UIControl_SaveList::addItem(const std::wstring& label, const std::wstring& textureName)
{
    addItem(label, textureName, 0);
    UIControl_List_addElement(this, 1, -1);
}

void UIControl_SaveList::setTextureName(int item, const std::wstring& textureName)
{
    FJ_List_ListButtonIconLeft_setTextureName(m_listRenderNode, item, textureName);
}

void UIControl_SaveList::addItem(const std::wstring& label)
{
    std::wstring textureName(L"px != 0");
    addItem(label, textureName);
}
