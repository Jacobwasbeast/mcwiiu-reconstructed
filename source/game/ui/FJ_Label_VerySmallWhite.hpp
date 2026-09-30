#ifndef GAME_UI_FJ_LABEL_VERYSMALLWHITE_HPP
#define GAME_UI_FJ_LABEL_VERYSMALLWHITE_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_VerySmallWhite : public FJ_Label
{
public:
    FJ_Label_VerySmallWhite(fuiRenderNode* pRenderNode);
    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
