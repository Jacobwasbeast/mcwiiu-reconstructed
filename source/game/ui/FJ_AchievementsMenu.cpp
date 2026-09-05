#include "game/ui/FJ_AchievementsMenu.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlTextPanelLarge.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_AchievementsMenu::FJ_AchievementsMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    FJ_FuiNode* moreDetails = getChildByName(std::string("GroupMoreDetails"));
    if (moreDetails != 0)
        moreDetails->setVisible(false);

    FJ_FuiNode* overlay = getChildByName(std::string("Overlay"));
    if (overlay != 0)
        overlay->setVisible(false);

    FJ_FuiNode* listContainer = getChildByName(std::string("AchievementsListContainer"))->asMovieClip();
    m_achievementsList = listContainer->getChildByName(std::string("AchievementsList"))->asFJ_List2D();
    m_achievementsList->SetScrollDelegate(&_PositionAchievementDescription, this);

    FJ_FuiNode* achievementName = getChildByName(std::string("AchievementName"));
    FJ_FuiNode* achievementDescription = getChildByName(std::string("AchievementDescription"));

    if (achievementDescription != 0)
    {
        const int fixedWidth = Math::round(achievementName->getWidth() / 3.0f);
        if (achievementDescription->isFJ_HtmlTextPanelLarge())
            achievementDescription->asFJ_HtmlTextPanelLarge()->SetFixedWidth(fixedWidth);
    }
}

FJ_AchievementsMenu::~FJ_AchievementsMenu()
{
}

void FJ_AchievementsMenu::PositionAchievementDescription()
{
    FJ_FuiNode* listContainer = getChildByName(std::string("AchievementsListContainer"));
    FJ_FuiNode* description = getChildByName(std::string("AchievementDescription"));
    FJ_FuiNode* activeItem = m_achievementsList->getActiveItem();
    const float rightLimit = static_cast<float>(getStage()->getStageWidth() - getISafeZoneRight());
    const float rightEdge = listContainer->getX() + activeItem->getX() + activeItem->getWidth() + description->getWidth();

    if (rightEdge < rightLimit)
    {
        fuiRect rect = description->getRect(activeItem);
        description->setX(static_cast<float>(Math::round(listContainer->getX() + activeItem->getX() + activeItem->getWidth() + rect.xMin)));
    }
    else
    {
        description->setX(static_cast<float>(Math::round(listContainer->getX() + activeItem->getX() - description->getWidth())));
    }

    const float y = listContainer->getY() + activeItem->getY();
    description->setY(static_cast<float>(Math::round(y + activeItem->getHeight() * 0.5f - description->getHeight() * 0.5f)));
}

void FJ_AchievementsMenu::_PositionAchievementDescription(FJ_FuiNode* node)
{
    static_cast<FJ_AchievementsMenu*>(node)->PositionAchievementDescription();
}

FJ_AchievementsMenu* FJ_AchievementsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_AchievementsMenu(renderNode);
}

unsigned int FJ_AchievementsMenu::getType()
{
    return 26;
}

void FJ_AchievementsMenu::SetAchievementDescription(const std::wstring& description)
{
    FJ_FuiNode* activeItem = m_achievementsList->getActiveItem();
    if (activeItem == 0)
        return;

    FJ_FuiNode* descriptionNode = getChildByName(std::string("AchievementDescription"));
    if (descriptionNode->isFJ_HtmlTextPanelLarge())
    {
        descriptionNode->asFJ_HtmlTextPanelLarge()->SetLabel(description);
    }
    else if (descriptionNode->isFJ_HtmlTextPanel())
    {
        descriptionNode->setVisible(!description.empty());
    }

    PositionAchievementDescription();
}

void FJ_AchievementsMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;

    setX(-static_cast<float>(right) - 176.0f);
    setY(-static_cast<float>(top) - 176.0f);
}
