#include "game/ui/FJ_Label_Book_Small.hpp"

FJ_Label_Book_Small::FJ_Label_Book_Small(fuiRenderNode* pRenderNode)
    : FJ_Label(pRenderNode)
{
    m_labelTooWide = true;
}

FJ_Label* FJ_Label_Book_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Book_Small(pRenderNode);
}

unsigned int FJ_Label_Book_Small::getType()
{
    return 0x8064;
}
