#include "game/ui/FJ_TouchControl.hpp"

FJ_TouchControl::FJ_TouchControl(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
    Init();
}

FJ_TouchControl* FJ_TouchControl::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_TouchControl(pRenderNode);
}

void FJ_TouchControl::Init()
{
    getChildByName("placeholder")->asMovieClip()->setHeight(0.0f);
}

unsigned int FJ_TouchControl::getType()
{
    return 0x65;
}

