#ifndef GAME_UI_FJ_LABEL_BOOK_VERYSMALL_HPP
#define GAME_UI_FJ_LABEL_BOOK_VERYSMALL_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Book_VerySmall : public FJ_Label
{
public:
    FJ_Label_Book_VerySmall(fuiRenderNode* pRenderNode);
    virtual ~FJ_Label_Book_VerySmall();

    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
