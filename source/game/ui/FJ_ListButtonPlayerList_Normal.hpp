#ifndef GAME_UI_FJ_LISTBUTTONPLAYERLIST_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONPLAYERLIST_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonPlayerList.hpp"

class fuiRenderNode;

class FJ_ListButtonPlayerList_Normal : public FJ_ListButtonPlayerList
{
public:
    explicit FJ_ListButtonPlayerList_Normal(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonPlayerList_Normal();
    static FJ_ListButtonPlayerList* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
