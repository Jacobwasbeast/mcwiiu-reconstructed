#ifndef GAME_UI_FJ_SOCIALPOST_HPP
#define GAME_UI_FJ_SOCIALPOST_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SocialPost : public FJ_Document
{
public:
    FJ_SocialPost(fuiRenderNode* renderNode);
    virtual ~FJ_SocialPost();
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
