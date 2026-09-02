#ifndef GAME_UI_FJ_EXTERNALHIGHLIGHT_HPP
#define GAME_UI_FJ_EXTERNALHIGHLIGHT_HPP

class FJ_FuiNode;
class fuiRenderNode;

class FJ_ExternalHighlight : public FJ_FuiNode
{
public:
    FJ_ExternalHighlight(fuiRenderNode* pRenderNode);

    virtual ~FJ_ExternalHighlight();

    static FJ_ExternalHighlight* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
