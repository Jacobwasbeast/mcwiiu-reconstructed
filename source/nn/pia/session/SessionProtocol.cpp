#include "nn/pia/session/SessionProtocol.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum SessionProtocolResult
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultSendBufferFull = 0x4c0d
};

enum
{
    cStationIndexNone = 0xfd,
    cTraceStationLevel = 0x4000000,
    cWindowSendCapacity = 2,
    cWindowReceiveCapacity = 2
};
}

SessionProtocol::SessionProtocol()
    : transport::Protocol(), m_stationIndex(cStationIndexNone), m_stationId(0), m_windowNum(0),
      m_windows(0), m_dispatchWindowIndex(0)
{
}

SessionProtocol::~SessionProtocol()
{
}

void SessionProtocol::Trace(unsigned long long value) const
{
}

unsigned short SessionProtocol::GetProtocolType() const
{
    return cProtocolType;
}

void SessionProtocol::Finalize()
{
    transport::ReliableSlidingWindow* windows = m_windows;
    if (windows != 0)
    {
        unsigned int count = pead::getArraySize(windows) / sizeof(transport::ReliableSlidingWindow);
        transport::ReliableSlidingWindow* window = windows;
        while (count != 0)
        {
            window->~ReliableSlidingWindow();
            ++window;
            --count;
        }
        pead::freeToSeadHeap(windows);
        m_windows = 0;
    }
    m_windowNum = 0;
}

Result SessionProtocol::Initialize(unsigned short stationNum)
{
    unsigned int count = static_cast<unsigned short>(stationNum - 1);
    m_windowNum = static_cast<unsigned short>(count);
    pead::Heap* heap = common::HeapManager::GetHeap();
    transport::ReliableSlidingWindow* windows = static_cast<transport::ReliableSlidingWindow*>(
        pead::allocateFromSeadHeap(count * sizeof(transport::ReliableSlidingWindow), heap));
    if (windows == 0)
    {
        m_windows = 0;
    }
    else
    {
        transport::ReliableSlidingWindow* window = windows;
        for (; count != 0; --count)
        {
            new (window) transport::ReliableSlidingWindow();
            ++window;
        }
        m_windows = windows;
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        Result result = m_windows[index].Initialize(cWindowSendCapacity, cWindowReceiveCapacity);
        if (result != 0)
        {
            Finalize();
            return result;
        }
    }
    return 0;
}

Result SessionProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    if (m_windows == 0)
        return cResultNotInitialized;
    if (m_stationIndex != cStationIndexNone)
        return cResultInvalidState;

    transport::Station* station = transport::g_piaStationManager->GetStation(address);
    if (station == 0)
        return cResultInvalidArgument;

    int stationIndex = static_cast<int>(station->m_field10);
    if (stationIndex < 0 || stationIndex > m_windowNum)
        return cResultInvalidArgument;

    m_stationId = (static_cast<unsigned long long>(station->m_field18) << 32) | station->m_field1c;
    m_dispatchWindowIndex = 0;
    m_stationIndex = stationIndex;
    return 0;
}

void SessionProtocol::CleanupWithStationAddress()
{
    if (m_stationIndex == cStationIndexNone)
        return;

    m_stationIndex = cStationIndexNone;
    m_stationId = 0;
    if (m_windowNum != 0)
    {
        unsigned int index = 0;
        do
        {
            if (m_windows[index].IsInCommunication())
                m_windows[index].Cleanup();
            ++index;
        } while (index < m_windowNum);
    }
}

Result SessionProtocol::CheckIsAbleToSendReliableMessage(StationIndex stationIndex,
                                                         unsigned int size)
{
    int windowIndex = static_cast<int>(stationIndex);
    if (m_stationIndex <= windowIndex)
        --windowIndex;

    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultNotInitialized;
    if (!window->CanPushData(size))
    {
        window->Trace(cTraceStationLevel);
        return cResultSendBufferFull;
    }
    return 0;
}

Result SessionProtocol::SendAnswerToInvitation(unsigned char processType,
                                               unsigned long long stationId, bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToInvitation, processType, stationId,
                                      result);
}

Result SessionProtocol::SendAnswerToDestroyInvitation(unsigned char processType,
                                                      unsigned long long stationId, bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToDestroyInvitation, processType,
                                      stationId, result);
}

Result SessionProtocol::SendAnswerToLeaveMeshInvitation(unsigned char processType,
                                                        unsigned long long stationId, bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToLeaveMeshInvitation, processType,
                                      stationId, result);
}

Result SessionProtocol::SendAnswerToCompletionInvitation(unsigned char processType,
                                                         unsigned long long stationId,
                                                         bool result)
{
    return SendAnswerToInvitationCore(cMessageTypeAnswerToCompletionInvitation, processType,
                                      stationId, result);
}

Result SessionProtocol::SendInvitationAsCompanion(unsigned char processType,
                                                  unsigned long long* stationIds,
                                                  unsigned short stationIdNum,
                                                  unsigned long long stationId,
                                                  unsigned long long* companionStationIds,
                                                  unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeInvitationAsCompanion, processType, stationIds,
                              stationIdNum, stationId, companionStationIds,
                              companionStationIdNum);
}

Result SessionProtocol::SendDestroyInvitation(unsigned char processType,
                                              unsigned long long* stationIds,
                                              unsigned short stationIdNum,
                                              unsigned long long stationId,
                                              unsigned long long* companionStationIds,
                                              unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeDestroyInvitation, processType, stationIds,
                              stationIdNum, stationId, companionStationIds,
                              companionStationIdNum);
}

Result SessionProtocol::SendLeaveMeshInvitation(unsigned char processType,
                                                unsigned long long* stationIds,
                                                unsigned short stationIdNum,
                                                unsigned long long stationId,
                                                unsigned long long* companionStationIds,
                                                unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeLeaveMeshInvitation, processType, stationIds,
                              stationIdNum, stationId, companionStationIds,
                              companionStationIdNum);
}

Result SessionProtocol::SendCompletionInvitation(unsigned char processType,
                                                 unsigned long long* stationIds,
                                                 unsigned short stationIdNum,
                                                 unsigned long long stationId,
                                                 unsigned long long* companionStationIds,
                                                 unsigned short companionStationIdNum)
{
    return SendInvitationCore(cMessageTypeCompletionInvitation, processType, stationIds,
                              stationIdNum, stationId, companionStationIds,
                              companionStationIdNum);
}

}}}
