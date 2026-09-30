#include "game/ui/FJ_DynamicHtmlText_WithPanel_Small.hpp"

FJ_DynamicHtmlText_WithPanel_Small::FJ_DynamicHtmlText_WithPanel_Small(fuiRenderNode* pRenderNode)
    : FJ_DynamicHtmlText(pRenderNode)
{
}

FJ_DynamicHtmlText* FJ_DynamicHtmlText_WithPanel_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_DynamicHtmlText_WithPanel_Small(pRenderNode);
}

unsigned int FJ_DynamicHtmlText_WithPanel_Small::getType()
{
    return 0x80ae;
}
