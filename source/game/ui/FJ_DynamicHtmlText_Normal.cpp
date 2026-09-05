#include "game/ui/FJ_DynamicHtmlText_Normal.hpp"

FJ_DynamicHtmlText_Normal::FJ_DynamicHtmlText_Normal(fuiRenderNode* pRenderNode)
    : FJ_DynamicHtmlText(pRenderNode)
{
}

FJ_DynamicHtmlText* FJ_DynamicHtmlText_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_DynamicHtmlText_Normal(pRenderNode);
}

FJ_DynamicHtmlText_Normal::~FJ_DynamicHtmlText_Normal()
{
}

unsigned int FJ_DynamicHtmlText_Normal::getType()
{
    return 0x80ab;
}
