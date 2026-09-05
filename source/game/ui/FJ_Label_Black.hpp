#ifndef GAME_UI_FJ_LABEL_BLACK_HPP
#define GAME_UI_FJ_LABEL_BLACK_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Black : public FJ_Label
{
public:
    explicit FJ_Label_Black(fuiRenderNode* renderNode);
    virtual ~FJ_Label_Black();
    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
