#include "game/ui/FJ_ExlamationMark.hpp"

FJ_ExlamationMark::FJ_ExlamationMark(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
}

FJ_FuiNode* FJ_ExlamationMark::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ExlamationMark(pRenderNode);
}

unsigned int FJ_ExlamationMark::getType()
{
    return 0x60;
}
