#include "game/ui/FJ_Label_BigGrey.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_BigGrey::FJ_Label_BigGrey(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label_BigGrey::~FJ_Label_BigGrey()
{
}

FJ_Label* FJ_Label_BigGrey::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_BigGrey(pRenderNode);
}

unsigned int FJ_Label_BigGrey::getType()
{
    return 0x805b;
}
