#include "game/ui/FJ_Label_VerySmallBlack.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_VerySmallBlack::FJ_Label_VerySmallBlack(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label_VerySmallBlack::~FJ_Label_VerySmallBlack()
{
}

FJ_Label* FJ_Label_VerySmallBlack::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_VerySmallBlack(renderNode);
}

unsigned int FJ_Label_VerySmallBlack::getType()
{
    return 0x8085;
}
