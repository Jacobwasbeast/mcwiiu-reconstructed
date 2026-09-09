#include "game/ui/FJ_Label_Book_VerySmall.hpp"

FJ_Label_Book_VerySmall::FJ_Label_Book_VerySmall(fuiRenderNode* pRenderNode)
    : FJ_Label(pRenderNode)
{
    m_labelTooWide = true;
}

FJ_Label* FJ_Label_Book_VerySmall::Create(fuiRenderNode* pRenderNode)
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
