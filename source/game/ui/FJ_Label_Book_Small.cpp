#include "game/ui/FJ_Label_Book_Small.hpp"

FJ_Label_Book_Small::FJ_Label_Book_Small(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
    m_labelData[0x8d] = 1;
}

FJ_Label_Book_Small::~FJ_Label_Book_Small()
{
}

int FJ_Label_Book_Small::Create(fuiRenderNode* pRenderNode)
{
    FJ_Label_Book_Small* label = new FJ_Label_Book_Small(pRenderNode);
    return (int)label;
}

unsigned int FJ_Label_Book_Small::getType()
{
    return 0x8064;
}
