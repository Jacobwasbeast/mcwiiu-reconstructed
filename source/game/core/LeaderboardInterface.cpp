#include "game/core/LeaderboardInterface.hpp"

#include "game/core/LeaderboardManager.hpp"
#include "game/network/LeaderboardReadListener.hpp"
#include "game/world/generation/PlayerUID.hpp"

LeaderboardInterface::LeaderboardInterface(LeaderboardManager* pManager)
    : m_pManager(pManager)
    , m_bRequestPending(false)
    , m_bSessionOpen(false)
    , m_eReadType(LeaderboardManager::eReadType_None)
    , m_pReadListener(NULL)
    , m_iUserIndex(0)
    , m_eStatsType(LeaderboardManager::eStatsType_Count)
    , m_uid(0)
    , m_uiStartRank(0)
    , m_uiNumRows(0)
{
    tryOpenSession();
}

LeaderboardInterface::~LeaderboardInterface()
{
    if (m_bSessionOpen)
    {
        m_bSessionOpen = false;
        m_pManager->CancelOperation();
        m_pManager->CloseSession();
    }
}

void LeaderboardInterface::tryOpenSession()
{
    if (!m_bSessionOpen)
    {
        m_bSessionOpen = m_pManager->OpenSession();
    }
}

bool LeaderboardInterface::callManager()
{
    if (!m_bSessionOpen)
    {
        return false;
    }

    switch (m_eReadType)
    {
    case LeaderboardManager::eReadType_Friends:
        return m_pManager->ReadStats_Friends(m_pReadListener, m_iUserIndex, m_eStatsType, m_uid, m_uiStartRank, m_uiNumRows);

    case LeaderboardManager::eReadType_MyScore:
        return m_pManager->ReadStats_MyScore(m_pReadListener, m_iUserIndex, m_eStatsType, m_uid, m_uiNumRows);

    case LeaderboardManager::eReadType_TopRank:
        return m_pManager->ReadStats_TopRank(m_pReadListener, m_iUserIndex, m_eStatsType, m_uiStartRank, m_uiNumRows);

    default:
        return true;
    }
}

void LeaderboardInterface::tick()
{
    tryOpenSession();

    if (m_bRequestPending)
    {
        m_bRequestPending = !callManager();
    }
}

void LeaderboardInterface::ReadStats_Friends(LeaderboardReadListener* pReadListener, int iUserIndex, LeaderboardManager::EStatsType eStatsType, PlayerUID uid, unsigned int uiStartRank, unsigned int uiNumRows)
{
    m_bRequestPending = true;
    m_eReadType = LeaderboardManager::eReadType_Friends;
    m_pReadListener = pReadListener;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_uid = uid;
    m_uiStartRank = uiStartRank;
    m_uiNumRows = uiNumRows;
    tick();
}

void LeaderboardInterface::ReadStats_MyScore(LeaderboardReadListener* pReadListener, int iUserIndex, LeaderboardManager::EStatsType eStatsType, PlayerUID uid, unsigned int uiNumRows)
{
    m_bRequestPending = true;
    m_eReadType = LeaderboardManager::eReadType_MyScore;
    m_pReadListener = pReadListener;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_uid = uid;
    m_uiNumRows = uiNumRows;
    tick();
}

void LeaderboardInterface::ReadStats_TopRank(LeaderboardReadListener* pReadListener, int iUserIndex, LeaderboardManager::EStatsType eStatsType, unsigned int uiStartRank, unsigned int uiNumRows)
{
    m_bRequestPending = true;
    m_eReadType = LeaderboardManager::eReadType_TopRank;
    m_pReadListener = pReadListener;
    m_iUserIndex = iUserIndex;
    m_eStatsType = eStatsType;
    m_uiStartRank = uiStartRank;
    m_uiNumRows = uiNumRows;

    tick();
}

void LeaderboardInterface::CancelOperation()
{
    m_pManager->CancelOperation();
    m_bRequestPending = false;
}
