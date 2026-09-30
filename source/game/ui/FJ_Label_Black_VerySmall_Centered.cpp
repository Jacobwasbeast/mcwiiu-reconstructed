#include "game/ui/FJ_Label_Black_VerySmall_Centered.hpp"

FJ_Label_Black_VerySmall_Centered::FJ_Label_Black_VerySmall_Centered(fuiRenderNode* pRenderNode)
    : FJ_Label(pRenderNode)
{
}

FJ_Label* FJ_Label_Black_VerySmall_Centered::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Black_VerySmall_Centered(pRenderNode);
}

unsigned int FJ_Label_Black_VerySmall_Centered::getType()
{
    return 0x8062;
}
