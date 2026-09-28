#include "game/core/LeaderboardManager.hpp"

#include <string.h>


LeaderboardManager::LeaderboardManager()
    : m_uid(0)
{
    zeroReadParameters();
    m_uid = PlayerUID(0);
}

void LeaderboardManager::zeroReadParameters()
{
    m_eReadType = eReadType_None;
    m_iUserIndex = -1;
    m_eStatsType = eStatsType_Count;
    m_pReadListener = NULL;
    m_uiStartRank = 0;
    m_uiNumRows = 0;
}

bool LeaderboardManager::ReadStats_Friends(LeaderboardReadListener* pReadListener, int iUserIndex, EStatsType eStatsType, PlayerUID uid, unsigned int uiStartRank, unsigned int uiNumRows)
{
    zeroReadParameters();

    m_eReadType = eReadType_Friends;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_pReadListener = pReadListener;

    return true;
}

bool LeaderboardManager::ReadStats_MyScore(LeaderboardReadListener* pReadListener, int iUserIndex, EStatsType eStatsType, PlayerUID uid, unsigned int uiNumRows)
{
    zeroReadParameters();

    m_eReadType = eReadType_MyScore;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_pReadListener = pReadListener;
    m_uiNumRows = uiNumRows;

    return true;
}

bool LeaderboardManager::ReadStats_TopRank(LeaderboardReadListener* pReadListener, int iUserIndex, EStatsType eStatsType, unsigned int uiStartRank, unsigned int uiNumRows)
{
    zeroReadParameters();

    m_eReadType = eReadType_TopRank;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_pReadListener = pReadListener;
    m_uiStartRank = uiStartRank;
    m_uiNumRows = uiNumRows;

    return true;
}

void LeaderboardManager::DeleteInstance()
{
    delete g_pLeaderboardManager;
    g_pLeaderboardManager = NULL;
}

void LeaderboardManager::printStats(ReadView& readView)
{
    for (unsigned int uiScore = 0; uiScore < readView.m_uiNumScores; ++uiScore)
    {
        ReadScore readScore(readView.m_pScores[uiScore]);
    }
}

LeaderboardManager::ReadScore::ReadScore(const ReadScore& other)
    : m_uid(other.m_uid)
    , m_uiRank(other.m_uiRank)
    , m_wsGamertag(other.m_wsGamertag)
    , m_uiRating(other.m_uiRating)
    , m_usNumStats(other.m_usNumStats)
    , m_iMessageStringId(other.m_iMessageStringId)
{
    memcpy(m_auiStats, other.m_auiStats, sizeof(m_auiStats));
}
