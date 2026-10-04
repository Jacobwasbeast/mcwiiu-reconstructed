#include "game/network/FakeNetworkPlayer.hpp"

#include <climits>

#include "game/core/System.hpp"

FakeNetworkPlayer::~FakeNetworkPlayer()
{
}

unsigned char FakeNetworkPlayer::GetSmallId()
{
    return mSmallId;
}

void FakeNetworkPlayer::SendData(INetworkPlayer* to, const void* data, int size, bool reliable, bool requiresAck)
{
}

bool FakeNetworkPlayer::IsSameSystem(INetworkPlayer* other)
{
    return other == this;
}

void FakeNetworkPlayer::UpdateBRUPQueue()
{
    if (!mBRUPQueue.empty())
    {
        unsigned int size = mBRUPQueue.front();
        uint64_t now = System::processTimeInMilliSecs();
        uint64_t elapsed = now - mLastBRUPTime;
        int elapsedHigh = static_cast<int>(elapsed >> 32);
        if (elapsedHigh < 0)
        {
            return;
        }
        if (elapsedHigh < 1 && static_cast<unsigned int>(elapsed) < size / 9)
        {
            return;
        }
        mBRUPQueue.pop_front();
        if (!mBRUPQueue.empty())
        {
            mLastBRUPTime = System::processTimeInMilliSecs();
            return;
        }
    }
    mLastBRUPTime = 0;
}

int FakeNetworkPlayer::GetOutstandingAckCount()
{
    UpdateBRUPQueue();
    return static_cast<int>(mBRUPQueue.size());
}

int FakeNetworkPlayer::GetSinglePlayerOutstandingAckCount()
{
    return 0;
}

void FakeNetworkPlayer::DecrementAcksOutstanding()
{
}

int FakeNetworkPlayer::GetSendQueueSizeBytes(INetworkPlayer* to, bool reliable)
{
    return 0;
}

int FakeNetworkPlayer::GetSendQueueSizeMessages(INetworkPlayer* to, bool reliable)
{
    return 0;
}

int FakeNetworkPlayer::GetCurrentRtt()
{
    return 0;
}

bool FakeNetworkPlayer::IsHost()
{
    return false;
}

bool FakeNetworkPlayer::IsGuest()
{
    return false;
}

bool FakeNetworkPlayer::IsLocal()
{
    return false;
}

int FakeNetworkPlayer::GetSessionIndex()
{
    return mSessionIndex;
}

bool FakeNetworkPlayer::IsTalking()
{
    return false;
}

bool FakeNetworkPlayer::IsMutedByLocalUser(int userIndex)
{
    return false;
}

bool FakeNetworkPlayer::HasVoice()
{
    return false;
}

bool FakeNetworkPlayer::HasCamera()
{
    return false;
}

int FakeNetworkPlayer::GetUserIndex()
{
    return mUserIndex;
}

void FakeNetworkPlayer::SetSocket(Socket* socket)
{
    mSocket = socket;
}

Socket* FakeNetworkPlayer::GetSocket()
{
    return mSocket;
}

const wchar_t* FakeNetworkPlayer::GetOnlineName()
{
    return mName.c_str();
}

std::wstring FakeNetworkPlayer::GetDisplayName()
{
    return mName;
}

PlayerUID FakeNetworkPlayer::GetUID()
{
    return mUID;
}

void FakeNetworkPlayer::SetUID(PlayerUID uid)
{
    mUID = uid;
}

void FakeNetworkPlayer::SentChunkPacket()
{
    mLastChunkPacketTime = System::processTimeInMilliSecs();
}

int FakeNetworkPlayer::GetTimeSinceLastChunkPacket_ms()
{
    if (mLastChunkPacketTime != 0)
    {
        return static_cast<int>(System::processTimeInMilliSecs() - mLastChunkPacketTime);
    }
    return INT_MAX;
}

bool FakeNetworkPlayer::isFake()
{
    return true;
}

IQNetPlayer* FakeNetworkPlayer::GetQNetPlayer()
{
    return NULL;
}
