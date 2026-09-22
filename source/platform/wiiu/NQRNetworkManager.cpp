#include "platform/wiiu/NQRNetworkManager.hpp"

#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"

int g_iGameBootInviteUserIndex = 0;
bool g_bSafeToRespondToGameBootInvite = false;

NQRNetworkManager::NQRNetworkManager()
    : m_stateChanges(), m_stateChangeLock(), m_playerLock(), m_localPlayerRequestLock(),
      m_localPlayerRequests()
{
}

NQRNetworkManager::~NQRNetworkManager()
{
    m_localPlayerRequests._Tidy();
    m_stateChanges._Tidy();
}

void NQRNetworkManager::SafeToRespondToGameBootInvite(int iUserIndex)
{
    g_iGameBootInviteUserIndex = iUserIndex;
    g_bSafeToRespondToGameBootInvite = true;
}

int NQRNetworkManager::GetOutstandingAckCount(NQRNetworkPlayer* pPlayer)
{
    int iOutstandingAcks = 0;
    int iPlayerCount = GetPlayerCount();

    int i = 0;
    for (; i + 7 < iPlayerCount; i += 8)
    {
        NQRNetworkPlayer* p0 = GetPlayerByIndex(i + 0);
        NQRNetworkPlayer* p1 = GetPlayerByIndex(i + 1);
        NQRNetworkPlayer* p2 = GetPlayerByIndex(i + 2);
        NQRNetworkPlayer* p3 = GetPlayerByIndex(i + 3);
        NQRNetworkPlayer* p4 = GetPlayerByIndex(i + 4);
        NQRNetworkPlayer* p5 = GetPlayerByIndex(i + 5);
        NQRNetworkPlayer* p6 = GetPlayerByIndex(i + 6);
        NQRNetworkPlayer* p7 = GetPlayerByIndex(i + 7);
        NQRNetworkPlayer* players[8] = {p0, p1, p2, p3, p4, p5, p6, p7};
        for (int j = 0; j < 8; ++j)
        {
            NQRNetworkPlayer* other = players[j];
            if (other != NULL && (pPlayer == other || pPlayer->IsSameSystem(other)))
                iOutstandingAcks += other->GetOutstandingAckCount();
        }
    }
    for (; i < iPlayerCount; ++i)
    {
        NQRNetworkPlayer* pOtherPlayer = GetPlayerByIndex(i);
        if (pOtherPlayer != NULL && (pPlayer == pOtherPlayer || pPlayer->IsSameSystem(pOtherPlayer)))
            iOutstandingAcks += pOtherPlayer->GetOutstandingAckCount();
    }

    return iOutstandingAcks;
}

int NQRNetworkManager::GetSendQueueSizeBytes()
{
    int iSendQueueBytes = 0;
    int iPlayerCount = GetPlayerCount();

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pPlayer = GetPlayerByIndex(i);

        if (pPlayer != NULL)
        {
            iSendQueueBytes += pPlayer->GetTotalSendQueueBytes();
        }
    }

    return iSendQueueBytes;
}

int NQRNetworkManager::GetSendQueueSizeMessages()
{
    int iSendQueueMessages = 0;
    int iPlayerCount = GetPlayerCount();

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pPlayer = GetPlayerByIndex(i);

        if (pPlayer != NULL)
        {
            iSendQueueMessages += pPlayer->GetTotalSendQueueMessages();
        }
    }

    return iSendQueueMessages;
}

NQRNetworkManager::PlayerSyncData::PlayerSyncData()
    : PlayerUID(0)
{
}
