#ifndef GAME_UI_FJ_HTMLTEXT_SMALL_HPP
#define GAME_UI_FJ_HTMLTEXT_SMALL_HPP

#include "game/ui/FJ_HtmlText.hpp"

class fuiRenderNode;

class FJ_HtmlText_Small : public FJ_HtmlText
{
private:
    fuiRenderNode* m_textField;

public:
    FJ_HtmlText_Small(fuiRenderNode* pRenderNode)
        : FJ_HtmlText(pRenderNode)
    {
    }

    virtual ~FJ_HtmlText_Small();
    static FJ_HtmlText* Create(fuiRenderNode* pRenderNode);
    virtual void ASConstructor();
    virtual unsigned int getType();
};

#endif
