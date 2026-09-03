#ifndef GAME_UI_FJ_LABEL_BOOK_SMALL_HPP
#define GAME_UI_FJ_LABEL_BOOK_SMALL_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_Label_Book_Small : public FJ_FuiNode
{
private:
    unsigned char m_labelData[0x118];

public:
    FJ_Label_Book_Small(fuiRenderNode* pRenderNode);
    virtual ~FJ_Label_Book_Small();
    static int Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
