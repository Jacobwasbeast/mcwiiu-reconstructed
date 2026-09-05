#include "game/ui/FJ_KeyboardButtons_Controls_Normal.hpp"

FJ_KeyboardButtons_Controls_Normal::FJ_KeyboardButtons_Controls_Normal(fuiRenderNode* renderNode)
    : FJ_KeyboardButtons(renderNode)
{
}

FJ_KeyboardButtons_Controls_Normal::~FJ_KeyboardButtons_Controls_Normal()
{
}

FJ_KeyboardButtons* FJ_KeyboardButtons_Controls_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_KeyboardButtons_Controls_Normal(renderNode);
}

unsigned int FJ_KeyboardButtons_Controls_Normal::getType()
{
    return 0x801a;
}
