#ifndef GAME_UI_FJ_LABEL_BOOK_NORMAL_HPP
#define GAME_UI_FJ_LABEL_BOOK_NORMAL_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Book_Normal : public FJ_Label
{
public:
    explicit FJ_Label_Book_Normal(fuiRenderNode* renderNode);
    virtual ~FJ_Label_Book_Normal();
    static int Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
