#include "game/ui/FJ_Label_SmallBlack.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_SmallBlack::FJ_Label_SmallBlack(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label_SmallBlack::~FJ_Label_SmallBlack()
{
}

FJ_Label* FJ_Label_SmallBlack::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SmallBlack(renderNode);
}

unsigned int FJ_Label_SmallBlack::getType()
{
    return 0x8080;
}
