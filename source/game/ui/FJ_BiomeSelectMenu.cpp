#include "game/ui/FJ_BiomeSelectMenu.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_BiomeSelectMenu::FJ_BiomeSelectMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
    FJ_FuiNode* guide = getChildByName(std::string("MainPanel.NavigatorGuide"));
    mNavigatorWidth = guide->getWidth();
    mNavigatorHeight = guide->getHeight();
    mNavigatorX = guide->getX();
    mNavigatorY = guide->getY();
}

FJ_AbstractContainerDocument* FJ_BiomeSelectMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_BiomeSelectMenu(renderNode);
}

unsigned int FJ_BiomeSelectMenu::getType()
{
    return 49;
}

void FJ_BiomeSelectMenu::SetMapNavigator(float scale, int x, int z)
{
    FJ_FuiNode* guide = getChildByName(std::string("MainPanel.NavigatorGuide"));
    guide->setWidth(static_cast<float>(Math::round(1.0f / scale * mNavigatorWidth)));
    guide->setHeight(static_cast<float>(Math::round(1.0f / scale * mNavigatorHeight)));

    const float guideWidth = guide->getWidth();
    const int offsetX = Math::round(mNavigatorWidth / 200.0f * static_cast<float>(x) + (mNavigatorWidth * 0.5f - guideWidth * 0.5f));
    const int offsetY = Math::round(mNavigatorWidth / 200.0f * static_cast<float>(z) + (mNavigatorWidth * 0.5f - guideWidth * 0.5f));
    guide->setX(mNavigatorX + static_cast<float>(offsetX));
    guide->setY(mNavigatorY + static_cast<float>(offsetY));

    if (guide->getX() + guideWidth > mNavigatorX + mNavigatorWidth)
    {
        guide->setX(mNavigatorX + mNavigatorWidth - guideWidth);
    }
    else if (guide->getX() < mNavigatorX)
    {
        guide->setX(mNavigatorX);
    }
    if (guide->getY() + guideWidth > mNavigatorY + mNavigatorWidth)
    {
        guide->setY(mNavigatorY + mNavigatorWidth - guideWidth);
    }
    else if (guide->getY() < mNavigatorY)
    {
        guide->setY(mNavigatorY);
    }
}
