#include "game/ui/FJ_Label_VerySmallGrey.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_VerySmallGrey::FJ_Label_VerySmallGrey(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label* FJ_Label_VerySmallGrey::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_VerySmallGrey(pRenderNode);
}

unsigned int FJ_Label_VerySmallGrey::getType()
{
    return 0x8086;
}
