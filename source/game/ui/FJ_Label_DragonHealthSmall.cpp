#include "game/ui/FJ_Label_DragonHealthSmall.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_DragonHealthSmall::FJ_Label_DragonHealthSmall(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label_DragonHealthSmall::~FJ_Label_DragonHealthSmall()
{
}

FJ_Label* FJ_Label_DragonHealthSmall::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_DragonHealthSmall(renderNode);
}

unsigned int FJ_Label_DragonHealthSmall::getType()
{
    return 0x8068;
}
