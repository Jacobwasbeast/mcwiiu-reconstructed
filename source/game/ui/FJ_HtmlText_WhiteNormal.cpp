#include "game/ui/FJ_HtmlText_WhiteNormal.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_HtmlText_WhiteNormal::FJ_HtmlText_WhiteNormal(fuiRenderNode* pRenderNode)
    : FJ_HtmlText(pRenderNode)
{
}

FJ_HtmlText_WhiteNormal::~FJ_HtmlText_WhiteNormal()
{
}

void FJ_HtmlText_WhiteNormal::ASConstructor()
{
    FJ_FuiNode* textContainer = getChildByName(std::string("FJ_TextContainer"));
    fuiRenderNode* textField = textContainer->getChildAt(0)->getRenderNodeChildByName(std::string("FJ_TextField"))->asEditText();
    m_pText = textField;
    addChild(m_pText);
    removeChild(textContainer, true);
}

FJ_HtmlText* FJ_HtmlText_WhiteNormal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HtmlText_WhiteNormal(pRenderNode);
}

unsigned int FJ_HtmlText_WhiteNormal::getType()
{
    return 0x80a4;
}
