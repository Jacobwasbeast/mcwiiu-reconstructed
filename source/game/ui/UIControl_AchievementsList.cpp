#include "game/ui/UIControl_AchievementsList.hpp"

#include "game/ui/FJ_List_ListButtonAchievement.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_AchievementsList::UIControl_AchievementsList()
{
}

UIControl_AchievementsList::~UIControl_AchievementsList()
{
}

bool UIControl_AchievementsList::setupControl(UIScene* scene, fuiRenderNode* node,
                                              const std::string& name)
{
    m_controlType = 31;
    return UIControl_GridList::setupControl(scene, node, name);
}

void UIControl_AchievementsList::AddNewItem(int id, const std::wstring& textureName)
{
    std::wstring itemTextureName(textureName);
    FJ_List_ListButtonAchievement* list = static_cast<FJ_List_ListButtonAchievement*>(
        m_fuiRenderNode->getFuiNode());
    list->addNewItem(id, itemTextureName);
    addElement(static_cast<UIControl::eUIControlType>(1), -1);
}
