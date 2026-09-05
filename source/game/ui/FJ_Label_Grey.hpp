#ifndef GAME_UI_FJ_LABEL_GREY_HPP
#define GAME_UI_FJ_LABEL_GREY_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Grey : public FJ_Label
{
public:
    explicit FJ_Label_Grey(fuiRenderNode* renderNode);
    virtual ~FJ_Label_Grey();

    static FJ_Label* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
