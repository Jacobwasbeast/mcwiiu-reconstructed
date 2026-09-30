#ifndef GAME_UI_FJ_LABEL_DRAGONHEALTHSMALL_HPP
#define GAME_UI_FJ_LABEL_DRAGONHEALTHSMALL_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_DragonHealthSmall : public FJ_Label
{
public:
    explicit FJ_Label_DragonHealthSmall(fuiRenderNode* renderNode);
    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
