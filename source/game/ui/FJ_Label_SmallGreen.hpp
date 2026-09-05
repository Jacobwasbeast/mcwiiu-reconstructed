#ifndef GAME_UI_FJ_LABEL_SMALLGREEN_HPP
#define GAME_UI_FJ_LABEL_SMALLGREEN_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SmallGreen : public FJ_Label
{
public:
    explicit FJ_Label_SmallGreen(fuiRenderNode* renderNode);
    virtual ~FJ_Label_SmallGreen();
    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
