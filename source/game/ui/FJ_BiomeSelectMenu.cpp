#include "game/ui/FJ_BiomeSelectMenu.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

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
    if (guide == NULL || scale == 0.0f)
    {
        return;
    }
    float ratio = 1.0f / scale;
    float width = mNavigatorWidth * ratio;
    float height = mNavigatorHeight * ratio;
    guide->setWidth(width);
    guide->setHeight(height);
    guide->setX(mNavigatorX + (mNavigatorWidth - width) * 0.5f + (float)x * mNavigatorWidth / 128.0f);
    guide->setY(mNavigatorY + (mNavigatorHeight - height) * 0.5f + (float)z * mNavigatorHeight / 128.0f);
}
