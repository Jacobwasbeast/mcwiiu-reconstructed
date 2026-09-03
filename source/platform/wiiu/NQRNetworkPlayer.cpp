#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"

#include <cstring>
#include <cwchar>

NQRNetworkPlayer::NQRNetworkPlayer(NQRNetworkManager* manager, eNQRNetworkPlayerType type, bool local,
    unsigned long long stationId, int localPlayerIndex, PlayerUID* uid,
    const char* networkName, const wchar_t* miiName)
    : m_type(type), m_local(local), m_connectionFlags(0), m_localPlayerIndex(localPlayerIndex),
      m_stationId(stationId), m_manager(manager), m_customData(0), m_totalSendQueueMessages(0),
      m_totalSendQueueBytes(0), m_outstandingAckCount(0), m_sessionIndex(0), m_talkingTicks(0)
{
    std::memset(m_miiName, 0, sizeof(m_miiName));
    std::wcscpy(m_miiName, miiName);
    std::memset(m_networkName, 0, sizeof(m_networkName));
    std::strncpy(m_networkName, networkName, 21);
    std::memset(m_networkNameW, 0, sizeof(m_networkNameW));
    std::mbstowcs(m_networkNameW, networkName, 21);
    if (uid != NULL)
        m_uid = *uid;
    else
        m_uid = PlayerUID(0);
    InitializeCriticalSection(&m_voiceMutex);
    InitializeCriticalSection(&m_sendMutex);
}

NQRNetworkPlayer::~NQRNetworkPlayer()
{
    DeleteCriticalSection(&m_voiceMutex);
}

unsigned char NQRNetworkPlayer::GetSmallId() { return m_smallId; }
wchar_t* NQRNetworkPlayer::GetMiiName() { return m_miiName; }
char* NQRNetworkPlayer::GetNetworkName() { return m_networkName; }
wchar_t* NQRNetworkPlayer::GetNetworkNameW() { return m_networkNameW; }
bool NQRNetworkPlayer::IsLocal() { return m_local ? (m_type == eNQRNetworkPlayerType_Host || m_type == eNQRNetworkPlayerType_Local) : (m_type == eNQRNetworkPlayerType_Local); }
unsigned char NQRNetworkPlayer::IsRemote() { return (unsigned char)!IsLocal(); }
bool NQRNetworkPlayer::IsHost() { return m_type == eNQRNetworkPlayerType_Host; }
unsigned int NQRNetworkPlayer::GetLocalPlayerIndex() { return m_localPlayerIndex; }
bool NQRNetworkPlayer::IsSameSystem(NQRNetworkPlayer* player) { return m_stationId == player->m_stationId; }
unsigned int NQRNetworkPlayer::GetCustomDataValue() { return m_customData; }
void NQRNetworkPlayer::SetCustomDataValue(unsigned int value) { m_customData = value; }

unsigned int NQRNetworkPlayer::HasVoice()
{
    if (CMinecraftApp::sInstance.IsVoiceChatEnabled())
        return m_smallId != 0;
    return 0;
}

unsigned int NQRNetworkPlayer::IsTalking()
{
    if (HasVoice())
    {
        ++m_talkingTicks;
        if (NintendoVoiceChat_WiiU::IsTalking(this))
        {
            m_talkingTicks = 0;
            return 1;
        }
        if (m_talkingTicks < 8)
            return 1;
    }
    return 0;
}

unsigned int NQRNetworkPlayer::IsMutedByLocalUser(int userIndex) { return 0; }
PlayerUID NQRNetworkPlayer::GetUID() { return m_uid; }
void NQRNetworkPlayer::SetUID(PlayerUID uid) { m_uid = uid; }
bool NQRNetworkPlayer::IsReady() { return (m_connectionFlags & 3) == 3; }
unsigned int NQRNetworkPlayer::IsConnectionComplete() { return m_connectionFlags & 1; }
void NQRNetworkPlayer::ConnectionComplete() { m_connectionFlags |= 1; }
void NQRNetworkPlayer::SmallIdAllocated(unsigned char smallId) { m_smallId = smallId; m_connectionFlags |= 2; }

void NQRNetworkPlayer::SendInternal(NQRNetworkPlayer* player, const void* data, unsigned int size, AckFlags flags)
{
    unsigned char* packet = new unsigned char[size + 1];
    packet[0] = (unsigned char)flags;
    std::memcpy(packet + 1, data, size);
    m_manager->RemoteDataSend(this, player, packet, size + 1);
    if (flags == Ack_Return)
    {
        EnterCriticalSection(&m_sendMutex);
        ++m_outstandingAckCount;
        LeaveCriticalSection(&m_sendMutex);
    }
    delete[] packet;
}

void NQRNetworkPlayer::SendData(NQRNetworkPlayer* player, const void* data, unsigned int size, bool reliable)
{
    AckFlags flags = reliable ? Ack_Return : Ack_Request;
    if (!m_local)
    {
        if (m_type == eNQRNetworkPlayerType_Local && player->m_type == eNQRNetworkPlayerType_Host)
            SendInternal(player, data, size, flags);
    }
    else if ((m_type == eNQRNetworkPlayerType_Host && player->m_type == eNQRNetworkPlayerType_Local) ||
             (m_type == eNQRNetworkPlayerType_Local && player->m_type == eNQRNetworkPlayerType_Host))
        m_manager->LocalDataSend(this, player, data, size);
    else if (m_type == eNQRNetworkPlayerType_Host && player->m_type == eNQRNetworkPlayerType_Remote)
        SendInternal(player, data, size, flags);
}

int NQRNetworkPlayer::GetPacketDataSize() { return 0; }
void NQRNetworkPlayer::WriteDataPacket(const void* data, int size, AckFlags flags) { SendInternal(this, data, (unsigned int)size, flags); }
void NQRNetworkPlayer::WriteAck(NQRNetworkPlayer* player) { unsigned char packet = 3; m_manager->RemoteDataSend(this, player, &packet, 1); }
void NQRNetworkPlayer::ReadAckReturn(AckFlags flags) { EnterCriticalSection(&m_sendMutex); --m_outstandingAckCount; LeaveCriticalSection(&m_sendMutex); }
void NQRNetworkPlayer::ReadAckRequest(AckFlags flags, NQRNetworkPlayer* player) { if (flags == Ack_Return) WriteAck(player); }

int NQRNetworkPlayer::GetOutstandingAckCount()
{
    int total = 0;
    int count = m_manager->GetPlayerCount();
    for (int i = 0; i < count; ++i)
    {
        NQRNetworkPlayer* player = m_manager->GetPlayerByIndex(i);
        if (player != NULL && (player == this || IsSameSystem(player)))
            total += (int)player->m_outstandingAckCount;
    }
    return total;
}

int NQRNetworkPlayer::GetSendQueueSizeBytes()
{
    int total = 0;
    int count = m_manager->GetPlayerCount();
    for (int i = 0; i < count; ++i)
    {
        NQRNetworkPlayer* player = m_manager->GetPlayerByIndex(i);
        if (player != NULL) total += (int)player->GetTotalSendQueueBytes();
    }
    return total;
}

int NQRNetworkPlayer::GetSendQueueSizeMessages()
{
    int total = 0;
    int count = m_manager->GetPlayerCount();
    for (int i = 0; i < count; ++i)
    {
        NQRNetworkPlayer* player = m_manager->GetPlayerByIndex(i);
        if (player != NULL) total += (int)player->GetTotalSendQueueMessages();
    }
    return total;
}

int NQRNetworkPlayer::GetTotalOutstandingAckCount() { return m_outstandingAckCount; }
unsigned int NQRNetworkPlayer::GetTotalSendQueueBytes() { return m_totalSendQueueBytes; }
unsigned int NQRNetworkPlayer::GetTotalSendQueueMessages() { EnterCriticalSection(&m_voiceMutex); unsigned int value = m_totalSendQueueMessages; LeaveCriticalSection(&m_voiceMutex); return value; }
int NQRNetworkPlayer::GetSessionIndex() { return m_manager->GetSessionIndex(this); }
