#include "game/ui/FJ_Label_VerySmallWhite.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_VerySmallWhite::FJ_Label_VerySmallWhite(fuiRenderNode* pRenderNode)
    : FJ_Label(pRenderNode)
{
}

FJ_Label_VerySmallWhite::~FJ_Label_VerySmallWhite()
{
}

FJ_Label* FJ_Label_VerySmallWhite::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_VerySmallWhite(pRenderNode);
}

unsigned int FJ_Label_VerySmallWhite::getType()
{
    return 0x8087;
}
