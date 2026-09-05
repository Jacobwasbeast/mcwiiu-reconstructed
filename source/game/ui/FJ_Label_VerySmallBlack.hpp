#ifndef GAME_UI_FJ_LABEL_VERYSMALLBLACK_HPP
#define GAME_UI_FJ_LABEL_VERYSMALLBLACK_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_VerySmallBlack : public FJ_Label
{
public:
    explicit FJ_Label_VerySmallBlack(fuiRenderNode* renderNode);
    virtual ~FJ_Label_VerySmallBlack();

    static FJ_Label* Create(fuiRenderNode* renderNode);

    virtual unsigned int getType();
};

#endif
