#include "game/ui/FJ_BiomeSelectMenu.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_BiomeSelectMenu::FJ_BiomeSelectMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode), mNavigatorWidth(0.0f), mNavigatorHeight(0.0f),
      mNavigatorX(0.0f), mNavigatorY(0.0f)
{
    FJ_FuiNode* guide = getChildByName(std::string("MainPanel_NavigatorGuide"));
    if (guide != NULL)
    {
        mNavigatorWidth = guide->getWidth();
        mNavigatorHeight = guide->getHeight();
        mNavigatorX = guide->getX();
        mNavigatorY = guide->getY();
    }
}

FJ_BiomeSelectMenu::~FJ_BiomeSelectMenu()
{
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
    FJ_FuiNode* guide = getChildByName(std::string("MainPanel_NavigatorGuide"));
    guide->setWidth(static_cast<float>(Math::round(mNavigatorWidth / scale)));
    guide->setHeight(static_cast<float>(Math::round(mNavigatorHeight / scale)));

    const float guideWidth = guide->getWidth();
    const float halfWidthDelta = mNavigatorWidth * 0.5f - guideWidth * 0.5f;
    const float targetX = mNavigatorX + static_cast<float>(Math::round(
        mNavigatorWidth / 200.0f * static_cast<float>(x) + halfWidthDelta));
    const float targetY = mNavigatorY + static_cast<float>(Math::round(
        mNavigatorWidth / 200.0f * static_cast<float>(z) + halfWidthDelta));
    guide->setX(targetX);
    guide->setY(targetY);

    if (guide->getX() + guideWidth <= mNavigatorX + mNavigatorWidth)
    {
        if (guide->getX() < mNavigatorX)
        {
            guide->setX(mNavigatorX);
        }
        if (guide->getY() + guideWidth <= mNavigatorY + mNavigatorWidth)
        {
            if (guide->getY() < mNavigatorY)
            {
                guide->setY(mNavigatorY);
            }
            return;
        }
    }
    else
    {
        guide->setX(mNavigatorX + mNavigatorWidth - guideWidth);
        if (guide->getY() + guideWidth <= mNavigatorY + mNavigatorWidth)
        {
            if (guide->getY() < mNavigatorY)
            {
                guide->setY(mNavigatorY);
            }
            return;
        }
    }
    guide->setY(mNavigatorY + mNavigatorWidth - guideWidth);
}
