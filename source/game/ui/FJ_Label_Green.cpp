#include "game/ui/FJ_Label_Green.hpp"

FJ_Label_Green::FJ_Label_Green(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label_Green::~FJ_Label_Green()
{
}

FJ_Label* FJ_Label_Green::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Green(renderNode);
}

unsigned int FJ_Label_Green::getType()
{
    return 0x806a;
}
