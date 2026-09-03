#ifndef GAME_UI_FJ_LEADERBOARDHEADERNORMAL_HPP
#define GAME_UI_FJ_LEADERBOARDHEADERNORMAL_HPP

#include "game/ui/FJ_LeaderboardHeader.hpp"

class FJ_LeaderboardHeaderNormal : public FJ_LeaderboardHeader
{
public:
    FJ_LeaderboardHeaderNormal(fuiRenderNode* pRenderNode);
    virtual ~FJ_LeaderboardHeaderNormal();
    static FJ_LeaderboardHeader* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
