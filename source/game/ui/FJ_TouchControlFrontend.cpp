#include "game/ui/FJ_TouchControlFrontend.hpp"

#include <string>

FJ_TouchControlFrontend::FJ_TouchControlFrontend(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
    Init();
}

FJ_TouchControlFrontend* FJ_TouchControlFrontend::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_TouchControlFrontend(pRenderNode);
}

FJ_TouchControlFrontend::~FJ_TouchControlFrontend()
{
}

void FJ_TouchControlFrontend::Init()
{
    std::string name("placeholder");
    FJ_FuiNode* pNode = getChildByName(name);
    pNode->asMovieClip()->setVisible(false);
}

unsigned int FJ_TouchControlFrontend::getType()
{
    return 0x66;
}
