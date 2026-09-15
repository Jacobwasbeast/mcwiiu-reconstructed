#include "platform/wiiu/NQRNetworkManager.hpp"

#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"

int g_iGameBootInviteUserIndex = 0;
bool g_bSafeToRespondToGameBootInvite = false;

NQRNetworkManager::NQRNetworkManager()
    : m_stateChanges(), m_stateChangeLock(), m_playerLock(), m_localPlayerRequestLock(),
      m_localPlayerRequests(), m_ghsVTable(0x10463DBCu)
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
    int iPlayerCount = Dispatch<int>(31);

    int i = 0;
    for (; i + 7 < iPlayerCount; i += 8)
    {
        NQRNetworkPlayer* p0 = Dispatch<NQRNetworkPlayer*>(33, i + 0);
        NQRNetworkPlayer* p1 = Dispatch<NQRNetworkPlayer*>(33, i + 1);
        NQRNetworkPlayer* p2 = Dispatch<NQRNetworkPlayer*>(33, i + 2);
        NQRNetworkPlayer* p3 = Dispatch<NQRNetworkPlayer*>(33, i + 3);
        NQRNetworkPlayer* p4 = Dispatch<NQRNetworkPlayer*>(33, i + 4);
        NQRNetworkPlayer* p5 = Dispatch<NQRNetworkPlayer*>(33, i + 5);
        NQRNetworkPlayer* p6 = Dispatch<NQRNetworkPlayer*>(33, i + 6);
        NQRNetworkPlayer* p7 = Dispatch<NQRNetworkPlayer*>(33, i + 7);
        NQRNetworkPlayer* players[8] = {p0, p1, p2, p3, p4, p5, p6, p7};
        for (int j = 0; j < 8; ++j)
        {
            NQRNetworkPlayer* other = players[j];
            if (other != NULL && (pPlayer == other || pPlayer->IsSameSystem(other)))
                iOutstandingAcks += *reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(other) + 0x100);
        }
    }
    for (; i < iPlayerCount; ++i)
    {
        NQRNetworkPlayer* pOtherPlayer = Dispatch<NQRNetworkPlayer*>(33, i);
        if (pOtherPlayer != NULL && (pPlayer == pOtherPlayer || pPlayer->IsSameSystem(pOtherPlayer)))
            iOutstandingAcks += *reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(pOtherPlayer) + 0x100);
    }

    return iOutstandingAcks;
}

int NQRNetworkManager::GetSendQueueSizeBytes()
{
    int iSendQueueBytes = 0;
    int iPlayerCount = Dispatch<int>(31);

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pPlayer = Dispatch<NQRNetworkPlayer*>(33, i);

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
    int iPlayerCount = Dispatch<int>(31);

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pPlayer = Dispatch<NQRNetworkPlayer*>(33, i);

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
