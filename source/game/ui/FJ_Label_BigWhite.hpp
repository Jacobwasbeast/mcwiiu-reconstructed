#ifndef GAME_UI_FJ_LABEL_BIGWHITE_HPP
#define GAME_UI_FJ_LABEL_BIGWHITE_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_BigWhite : public FJ_Label
{
public:
    explicit FJ_Label_BigWhite(fuiRenderNode* renderNode);
    virtual ~FJ_Label_BigWhite();

    static FJ_Label* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
