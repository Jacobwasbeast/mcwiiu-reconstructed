#include "game/ui/FJ_List_ListButtonAchievement.hpp"

#include <cstdio>

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_ListButtonAchievement.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_List_ListButtonAchievement::FJ_List_ListButtonAchievement(fuiRenderNode* renderNode)
    : FJ_List2D(renderNode)
{
}

FJ_List_ListButtonAchievement::~FJ_List_ListButtonAchievement()
{
}

void FJ_List_ListButtonAchievement::addNewItem(int id, const std::wstring& textureName)
{
    char name[64];
    std::sprintf(name, "AchievementButton_%d", id);

    const char* objectName = !m_outline
        ? "fourj.Buttons.FJ_ListButtonAchievement_Normal"
        : "fourj.Buttons.FJ_ListButtonAchievement_Small";
    FJ_ListButtonAchievement* item = static_cast<FJ_ListButtonAchievement*>(
        FJ_Factory::createObject(objectName, m_pRenderNode, true, name));
    addNewListItem(item, id);
    item->Init(id, textureName);
}

FJ_List2D* FJ_List_ListButtonAchievement::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_ListButtonAchievement(renderNode);
}

uint32_t FJ_List_ListButtonAchievement::getType()
{
    return 0x8035;
}
