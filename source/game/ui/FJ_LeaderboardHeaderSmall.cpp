#include "game/ui/FJ_LeaderboardHeaderSmall.hpp"

FJ_LeaderboardHeaderSmall::FJ_LeaderboardHeaderSmall(fuiRenderNode* pRenderNode)
    : FJ_LeaderboardHeader(pRenderNode)
{
}

FJ_LeaderboardHeaderSmall::~FJ_LeaderboardHeaderSmall()
{
}

FJ_LeaderboardHeader* FJ_LeaderboardHeaderSmall::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardHeaderSmall(pRenderNode);
}

unsigned int FJ_LeaderboardHeaderSmall::getType()
{
    return 0x6b;
}
