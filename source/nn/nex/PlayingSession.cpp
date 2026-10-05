#include "nn/nex/PlayingSession.hpp"

#include "nn/nex/MatchmakeSession.hpp"

namespace nn
{
namespace nex
{

PlayingSession::~PlayingSession()
{
}

MatchmakeSession* PlayingSession::GetMatchmakeSession() const
{
    Gathering* pGathering = m_Gathering.GetObject();
    if (pGathering != 0 && pGathering->IsAKindOf(String(L"MatchmakeSession")))
        return static_cast<MatchmakeSession*>(pGathering);
    return 0;
}

}
}
