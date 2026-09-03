#ifndef GAME_UI_FJ_LABEL_BOOK_VERYSMALL_HPP
#define GAME_UI_FJ_LABEL_BOOK_VERYSMALL_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_Label_Book_VerySmall : public FJ_FuiNode
{
public:
    FJ_Label_Book_VerySmall(fuiRenderNode* pRenderNode);
    virtual ~FJ_Label_Book_VerySmall();

    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
