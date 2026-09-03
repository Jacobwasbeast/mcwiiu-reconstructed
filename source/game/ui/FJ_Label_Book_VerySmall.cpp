#include "game/ui/FJ_Label_Book_VerySmall.hpp"

FJ_Label_Book_VerySmall::FJ_Label_Book_VerySmall(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
}

FJ_FuiNode* FJ_Label_Book_VerySmall::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Book_VerySmall(pRenderNode);
}

FJ_Label_Book_VerySmall::~FJ_Label_Book_VerySmall()
{
}

unsigned int FJ_Label_Book_VerySmall::getType()
{
    return 0x8065;
}
