#ifndef GAME_UI_FJ_LABEL_BIGGREY_HPP
#define GAME_UI_FJ_LABEL_BIGGREY_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_BigGrey : public FJ_Label
{
public:
    explicit FJ_Label_BigGrey(fuiRenderNode* renderNode);
    virtual ~FJ_Label_BigGrey();

    static FJ_Label* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
