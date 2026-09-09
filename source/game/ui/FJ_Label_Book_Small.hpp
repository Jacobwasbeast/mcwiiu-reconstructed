#ifndef GAME_UI_FJ_LABEL_BOOK_SMALL_HPP
#define GAME_UI_FJ_LABEL_BOOK_SMALL_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Book_Small : public FJ_Label
{
public:
    explicit FJ_Label_Book_Small(fuiRenderNode* pRenderNode);
    virtual ~FJ_Label_Book_Small();
    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
