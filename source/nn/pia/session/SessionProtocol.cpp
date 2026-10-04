#include "nn/pia/session/SessionProtocol.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum SessionProtocolResult
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultSendBufferFull = 0x4c0d,
    cResultNoInvitationTarget = 0x2c0a,
    cResultInvitationSendFailed = 0x8c56,
    cResultSessionProcessFailed = 0x6c59
};

enum
{
    cStationIndexNone = 0xfd,
    cStationNumMax = transport::StationManager::cStationNumMax,
    cStationIndexMax = cStationNumMax - 1,
    cStationStateConnected = 5,
    cTraceStationLevel = 0x4000000,
    cWindowSendCapacity = 2,
    cWindowReceiveCapacity = 2,
    cStationIdSize = 8,
    cMessageTypeOffset = 0,
    cProcessTypeOffset = 1,
    cAnswerResultOffset = 2,
    cAnswerStationIdOffset = 3,
    cAnswerHeaderSize = 3,
    cInvitationStationIdOffset = 2,
    cInvitationStationIdNumOffset = 10,
    cInvitationStationIdListOffset = 11,
    cInvitationHeaderSize = 3,
    cInvitationStationIdMax = 0x20,
    cPreparedResultOffset = 2,
    cPreparedSessionIdOffset = 3,
    cPreparedStationIdOffset = 7,
    cPreparedHeaderSize = 7,
    cCompletionFailureStationIdOffset = 2,
    cCompletionFailureHeaderSize = 2,
    cCompletionMigrateResultOffset = 2,
    cCompletionMigrateStationIdOffset = 3,
    cCompletionMigrateHeaderSize = 3,
    cNextSessionIdSessionIdOffset = 2,
    cNextSessionIdNextSessionIdOffset = 6,
    cNextSessionIdStationIdOffset = 10,
    cNextSessionIdSessionIndexLength = 1,
    cNextSessionIdOwnerLength = 8,
    cNextSessionIdReservedLength = 1,
    cNextSessionIdApplicationDataSizeLength = 1,
    cNextSessionIdParseOwnerOffset = 0x13,
    cNextSessionIdParseApplicationDataSizeOffset = 0x1c,
    cNextSessionIdParseApplicationDataOffset = 0x1d,
    cNextSessionIdParseSessionIndexOffset = 0x12
};

const unsigned int cUnlimitedDispatchSize = 0xffffffffu;

inline transport::StationPacketHandler* handlerOf(unsigned int value)
{
    return reinterpret_cast<transport::StationPacketHandler*>(value);
}
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

unsigned long long SessionProtocol::DeserializeStationId(const unsigned char* buffer)
{
    return common::deserializeU64(buffer);
}

unsigned int SessionProtocol::SerializeStationId(unsigned long long stationId,
                                                 unsigned char* buffer, unsigned int bufferSize)
{
    if (bufferSize < cStationIdSize)
        return 0;
    common::serializeU64(buffer, stationId);
    return cStationIdSize;
}

Result SessionProtocol::SendPreparedForMigrateSession(unsigned char processType,
                                                      unsigned long long stationId,
                                                      unsigned int sessionId, bool value)
{
    transport::StationIdTable::Entry entry;
    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    Result result = table->FindByStationIndex(&entry, static_cast<StationIndex>(m_stationIndex));
    if (result != 0)
        return result;
    unsigned long long localStationId = entry.stationId;
    result = table->FindByStationId(&entry, stationId);
    if (result != 0)
        return result;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (m_stationIndex <= windowIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypePreparedForMigrateSession;
    m_messageBuffer[cPreparedResultOffset] = value;
    common::serializeU32(m_messageBuffer + cPreparedSessionIdOffset, sessionId);
    unsigned int size = SerializeStationId(localStationId,
                                           m_messageBuffer + cPreparedStationIdOffset,
                                           sizeof(m_messageBuffer) - cPreparedStationIdOffset);
    if (sessionId == 0 || size == 0)
        return cResultInvalidState;
    return window->PushData(m_messageBuffer, size + cPreparedHeaderSize);
}

Result SessionProtocol::SendAnswerToInvitationCore(MessageType messageType,
                                                   unsigned char processType,
                                                   unsigned long long stationId, bool result)
{
    transport::StationIdTable::Entry entry;
    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    Result findResult = table->FindByStationIndex(&entry, static_cast<StationIndex>(m_stationIndex));
    if (findResult != 0)
        return findResult;
    unsigned long long localStationId = entry.stationId;
    findResult = table->FindByStationId(&entry, stationId);
    if (findResult != 0)
        return findResult;
    if (entry.stationIndex == cStationIndexNone)
        return cResultInvalidState;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (m_stationIndex <= windowIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cMessageTypeOffset] = messageType;
    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cAnswerResultOffset] = result;
    unsigned int size = SerializeStationId(localStationId, m_messageBuffer + cAnswerStationIdOffset,
                                           sizeof(m_messageBuffer) - cAnswerStationIdOffset);
    if (size == 0)
        return cResultInvalidState;
    return window->PushData(m_messageBuffer, size + cAnswerHeaderSize);
}

Result SessionProtocol::SendInvitationCore(MessageType messageType, unsigned char processType,
                                           unsigned long long* stationIds,
                                           unsigned short stationIdNum,
                                           unsigned long long stationId,
                                           unsigned long long* companionStationIds,
                                           unsigned short companionStationIdNum)
{
    m_messageBuffer[cMessageTypeOffset] = messageType;
    m_messageBuffer[cProcessTypeOffset] = processType;
    unsigned int size = SerializeStationId(stationId, m_messageBuffer + cInvitationStationIdOffset,
                                           sizeof(m_messageBuffer) - cInvitationStationIdOffset);
    m_messageBuffer[cInvitationStationIdOffset + size] = static_cast<unsigned char>(stationIdNum);
    unsigned int messageSize = size + cInvitationHeaderSize;
    if (size == 0 || stationIdNum > cInvitationStationIdMax)
        return cResultInvalidState;

    for (unsigned int index = 0; index < stationIdNum; ++index)
    {
        unsigned int written = SerializeStationId(stationIds[index], m_messageBuffer + messageSize,
                                                  sizeof(m_messageBuffer) - messageSize);
        if (written == 0)
            return cResultInvalidState;
        messageSize += written;
    }

    const unsigned long long* targets = companionStationIds;
    unsigned int targetNum = companionStationIdNum;
    if (targets == 0)
    {
        targets = stationIds;
        targetNum = stationIdNum;
    }

    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    transport::ReliableSlidingWindow* windows[cStationNumMax] = { 0 };
    unsigned short windowNum = 0;
    Result result;
    for (unsigned int index = 0; index < targetNum; ++index)
    {
        if (targets[index] == stationId)
            continue;
        transport::StationIdTable::Entry entry;
        result = table->FindByStationId(&entry, targets[index]);
        if (result != 0 || static_cast<int>(entry.stationIndex) > cInvitationStationIdMax)
            continue;
        int windowIndex = static_cast<int>(entry.stationIndex);
        if (m_stationIndex <= windowIndex)
            --windowIndex;
        transport::ReliableSlidingWindow* window = m_windows + windowIndex;
        windows[windowNum] = window;
        if (!window->IsInCommunication())
        {
            result = cResultNotInitialized;
            continue;
        }
        if (!window->CanPushData(messageSize))
        {
            window->Trace(cTraceStationLevel);
            result = cResultSendBufferFull;
        }
        else
        {
            result = 0;
        }
        if (result == cResultSendBufferFull)
            return result;
        g_piaSession->m_stationIdStatusTable->SetSendInvitationStatus(targets[index], true);
        ++windowNum;
    }

    if (windowNum == 0)
        return cResultNoInvitationTarget;

    unsigned short sentNum = 0;
    for (unsigned short index = 0; index < windowNum; ++index)
    {
        result = windows[index]->PushData(m_messageBuffer, messageSize);
        if (result == 0)
            ++sentNum;
    }
    if (sentNum != 0)
        return 0;
    return cResultInvitationSendFailed;
}

Result SessionProtocol::SendNextSessionId(unsigned char processType, unsigned int sessionId,
                                          unsigned int nextSessionId,
                                          unsigned long long matchmakeOwnerUniqueId,
                                          const unsigned char* applicationData,
                                          unsigned int applicationDataSize,
                                          unsigned long long* stationIds,
                                          unsigned short stationIdNum)
{
    transport::StationIdTable::Entry entry;
    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    Result result = table->FindByStationIndex(&entry, static_cast<StationIndex>(m_stationIndex));
    if (result != 0)
        return result;
    unsigned long long localStationId = entry.stationId;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypeNextSessionId;
    common::serializeU32(m_messageBuffer + cNextSessionIdSessionIdOffset, sessionId);
    common::serializeU32(m_messageBuffer + cNextSessionIdNextSessionIdOffset, nextSessionId);
    unsigned int size = SerializeStationId(localStationId,
                                           m_messageBuffer + cNextSessionIdStationIdOffset,
                                           sizeof(m_messageBuffer) - cNextSessionIdStationIdOffset);

    unsigned int sessionIndexOffset = cNextSessionIdStationIdOffset + size;
    Session* session = g_piaSession;
    if (processType >= Session::cProcessTypeCreateJointSession &&
        processType <= Session::cProcessTypeJoinRandomJointSession)
    {
        unsigned int bufferIndex = session->m_currentSessionIndex == 0 ? 1 : 0;
        m_messageBuffer[sessionIndexOffset] = static_cast<unsigned char>(session->m_fieldc2[bufferIndex]);
    }
    else
    {
        m_messageBuffer[sessionIndexOffset] =
            static_cast<unsigned char>(session->m_fieldc2[session->m_currentSessionIndex]);
    }
    common::serializeU64(m_messageBuffer + sessionIndexOffset + cNextSessionIdSessionIndexLength,
                         matchmakeOwnerUniqueId);
    unsigned int applicationDataSizeOffset =
        sessionIndexOffset + cNextSessionIdSessionIndexLength + cNextSessionIdOwnerLength +
        cNextSessionIdReservedLength;
    m_messageBuffer[applicationDataSizeOffset] = static_cast<unsigned char>(applicationDataSize);
    unsigned int messageSize = applicationDataSizeOffset + cNextSessionIdApplicationDataSizeLength;
    for (unsigned int index = 0; index < applicationDataSize; ++index)
    {
        m_messageBuffer[messageSize] = applicationData[index];
        ++messageSize;
    }

    if (sessionId == 0 || nextSessionId == 0 || size == 0)
        return cResultInvalidState;

    transport::ReliableSlidingWindow* windows[cStationNumMax] = { 0 };
    unsigned short windowNum = 0;
    for (unsigned int index = 0; index < stationIdNum; ++index)
    {
        if (stationIds[index] == localStationId)
            continue;
        result = table->FindByStationId(&entry, stationIds[index]);
        if (result != 0 || static_cast<int>(entry.stationIndex) > cInvitationStationIdMax)
            continue;
        int windowIndex = static_cast<int>(entry.stationIndex);
        if (m_stationIndex <= windowIndex)
            --windowIndex;
        transport::ReliableSlidingWindow* window = m_windows + windowIndex;
        windows[windowNum] = window;
        if (!window->IsInCommunication())
        {
            result = cResultNotInitialized;
            continue;
        }
        if (!window->CanPushData(messageSize))
        {
            window->Trace(cTraceStationLevel);
            result = cResultSendBufferFull;
        }
        else
        {
            result = 0;
        }
        if (result == cResultSendBufferFull)
            return result;
        session->m_stationIdStatusTable->SetSendInvitationStatus(stationIds[index], true);
        ++windowNum;
    }

    if (windowNum == 0)
        return cResultNoInvitationTarget;

    unsigned short sentNum = 0;
    for (unsigned short index = 0; index < windowNum; ++index)
    {
        result = windows[index]->PushData(m_messageBuffer, messageSize);
        if (result == 0)
            ++sentNum;
    }
    if (sentNum != 0)
        return 0;
    return cResultInvitationSendFailed;
}

Result SessionProtocol::SendCompletionFailureResponse(unsigned char processType,
                                                      unsigned long long stationId)
{
    transport::StationIdTable::Entry entry;
    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    unsigned long long localStationId;
    {
        Result result = table->FindByStationIndex(&entry, static_cast<StationIndex>(m_stationIndex));
        if (result != 0)
            return result;
        localStationId = entry.stationId;
    }
    Result result = table->FindByStationId(&entry, stationId);
    if (result != 0)
        return result;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (m_stationIndex <= windowIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypeCompletionFailureResponse;
    unsigned int size = SerializeStationId(localStationId,
                                           m_messageBuffer + cCompletionFailureStationIdOffset,
                                           sizeof(m_messageBuffer) -
                                               cCompletionFailureStationIdOffset);
    return window->PushData(m_messageBuffer, size + cCompletionFailureHeaderSize);
}

Result SessionProtocol::SendCompletionForMigrateSession(unsigned char processType,
                                                        unsigned long long stationId, bool result)
{
    transport::StationIdTable::Entry entry;
    transport::StationIdTable* table = transport::g_piaTransport->m_stationIdTable;
    Result findResult = table->FindByStationIndex(&entry, static_cast<StationIndex>(m_stationIndex));
    if (findResult != 0)
        return findResult;
    unsigned long long localStationId = entry.stationId;
    findResult = table->FindByStationId(&entry, stationId);
    if (findResult != 0)
        return findResult;

    int windowIndex = static_cast<int>(entry.stationIndex);
    if (m_stationIndex <= windowIndex)
        --windowIndex;
    transport::ReliableSlidingWindow* window = m_windows + windowIndex;
    if (!window->IsInCommunication())
        return cResultInvalidState;

    m_messageBuffer[cProcessTypeOffset] = processType;
    m_messageBuffer[cMessageTypeOffset] = cMessageTypeCompletionForMigrateSession;
    m_messageBuffer[cCompletionMigrateResultOffset] = result;
    unsigned int size = SerializeStationId(localStationId,
                                           m_messageBuffer + cCompletionMigrateStationIdOffset,
                                           sizeof(m_messageBuffer) -
                                               cCompletionMigrateStationIdOffset);
    return window->PushData(m_messageBuffer, size + cCompletionMigrateHeaderSize);
}

void SessionProtocol::ParseNextSessionId(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned int sessionId = common::deserializeU32(data + cNextSessionIdSessionIdOffset);
    unsigned int nextSessionId = common::deserializeU32(data + cNextSessionIdNextSessionIdOffset);
    unsigned long long stationId = DeserializeStationId(data + cNextSessionIdStationIdOffset);
    unsigned char sessionIndex = data[cNextSessionIdParseSessionIndexOffset];
    unsigned long long matchmakeOwnerUniqueId = 0;
    unsigned char applicationDataSize = 0;
    const unsigned char* applicationData = 0;
    if (message.m_messageSize > cNextSessionIdParseSessionIndexOffset)
    {
        matchmakeOwnerUniqueId = common::deserializeU64(data + cNextSessionIdParseOwnerOffset);
        applicationDataSize = data[cNextSessionIdParseApplicationDataSizeOffset];
        applicationData = data + cNextSessionIdParseApplicationDataOffset;
    }

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0 || sessionIndex == 0)
        return;

    Session* session = g_piaSession;
    if (sessionId != session->m_sessionId[session->m_currentSessionIndex])
        return;
    JointSessionJob* job = session->m_jointSessionJob;
    if (job == 0)
        return;

    if (job->m_processType == 0)
    {
        SendPreparedForMigrateSession(processType, stationId, nextSessionId, false);
    }
    else if (job->m_field60 == messageType)
    {
        unsigned int bufferIndex = session->m_currentSessionIndex == 0 ? 1 : 0;
        session->m_fieldc2[bufferIndex] = sessionIndex;
        job->ReceiveNextSessionId(static_cast<Session::ProcessType>(processType), nextSessionId,
                                  matchmakeOwnerUniqueId, applicationData, applicationDataSize,
                                  stationId);
    }
}

void SessionProtocol::ParsePreparedForMigrateSession(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char prepared = data[cPreparedResultOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned int sessionId = common::deserializeU32(data + cPreparedSessionIdOffset);
    unsigned long long stationId = DeserializeStationId(data + cPreparedStationIdOffset);

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return;

    Session* session = g_piaSession;
    unsigned int expectedSessionId;
    if (processType == Session::cProcessTypeDestroyJointSession ||
        processType == Session::cProcessTypeLeaveJointSession)
        expectedSessionId = session->m_sessionId[session->m_currentSessionIndex];
    else
        expectedSessionId = session->m_sessionId[session->m_currentSessionIndex == 0 ? 1 : 0];
    if (sessionId == expectedSessionId)
        session->m_stationIdStatusTable->SetPrepareStatus(stationId, prepared != 0);
}

void SessionProtocol::ParseInvitationAsCompanion(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    unsigned int stationIdNum = data[cInvitationStationIdNumOffset];
    unsigned int offset = cInvitationStationIdListOffset;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0)
        return;
    if (job->m_fielda2)
        return;

    unsigned long long stationIds[cInvitationStationIdMax];
    Session::ProcessType currentProcess = job->m_processType;
    if (currentProcess == 0)
    {
        if (hostStationId == message.m_sourceStationId && stationIdNum <= cInvitationStationIdMax)
        {
            for (unsigned int index = 0; index < stationIdNum; ++index)
            {
                stationIds[index] = DeserializeStationId(data + offset);
                offset += cStationIdSize;
            }
            Result result = job->StartupPassiveProcess(
                static_cast<Session::ProcessType>(processType),
                static_cast<MessageType>(messageType), stationIds,
                static_cast<unsigned short>(stationIdNum), hostStationId);
            if (result == 0)
            {
                job->Ready(false);
                return;
            }
            currentProcess = job->m_processType;
        }
    }
    else
    {
        if (currentProcess != processType)
        {
            job->m_fielda2 = 1;
            job->m_fielda4 = cResultSessionProcessFailed;
            return;
        }
        if (processType != Session::cProcessTypeDestroyJointSession || job->m_leaveStationId != 0)
        {
            if (hostStationId == job->m_leaveStationId)
            {
                SendAnswerToInvitation(processType, hostStationId, true);
                return;
            }
        }
        else if (stationIdNum <= cInvitationStationIdMax)
        {
            for (unsigned int index = 0; index < stationIdNum; ++index)
            {
                stationIds[index] = DeserializeStationId(data + offset);
                offset += cStationIdSize;
            }
            Result result = job->ReceiveLeaveInvitation(
                Session::cProcessTypeDestroyJointSession, stationIds,
                static_cast<unsigned short>(stationIdNum), hostStationId);
            if (result == 0)
                return;
            currentProcess = job->m_processType;
        }
    }

    if (currentProcess != 0)
    {
        job->m_fielda2 = 1;
        job->m_fielda4 = cResultSessionProcessFailed;
        return;
    }
    if (g_piaSession->IsProcessingJoin())
        g_piaSession->CancelJoinProcessByFatalError();
    else
        g_piaSession->ChangeStatusToDisconnectedSession();
}

void SessionProtocol::ParseDestroyInvitation(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    unsigned int offset = cInvitationStationIdListOffset;
    unsigned int stationIdNum = data[cInvitationStationIdNumOffset];
    if (processType != Session::cProcessTypeDestroyJointSession)
        return;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0 || job->m_fielda2)
        return;

    unsigned long long stationIds[cInvitationStationIdMax];
    Session::ProcessType currentProcess = job->m_processType;
    if (currentProcess == 0)
    {
        if (hostStationId == message.m_sourceStationId && stationIdNum <= cInvitationStationIdMax)
        {
            for (unsigned int index = 0; index < stationIdNum; ++index)
            {
                stationIds[index] = DeserializeStationId(data + offset);
                offset += cStationIdSize;
            }
            Result result = job->StartupPassiveProcess(
                Session::cProcessTypeDestroyJointSession, static_cast<MessageType>(messageType),
                stationIds, static_cast<unsigned short>(stationIdNum), hostStationId);
            if (result == 0)
            {
                job->Ready(false);
                return;
            }
            currentProcess = job->m_processType;
        }
    }
    else if (currentProcess == Session::cProcessTypeDestroyJointSession ||
             currentProcess == Session::cProcessTypeLeaveJointSession)
    {
        if (g_piaSession->GetJointSessionHostStationId() != hostStationId)
            return;
        SendAnswerToDestroyInvitation(Session::cProcessTypeDestroyJointSession, hostStationId,
                                      true);
        return;
    }

    if (currentProcess != 0)
    {
        job->m_fielda2 = 1;
        job->m_fielda4 = cResultSessionProcessFailed;
        return;
    }
    if (g_piaSession->IsProcessingJoin())
        g_piaSession->CancelJoinProcessByFatalError();
    else
        g_piaSession->ChangeStatusToDisconnectedSession();
}

void SessionProtocol::ParseLeaveMeshInvitation(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    unsigned int stationIdNum = data[cInvitationStationIdNumOffset];
    unsigned int offset = cInvitationStationIdListOffset;
    unsigned long long stationIds[cInvitationStationIdMax];
    for (unsigned int index = 0; index < stationIdNum; ++index)
    {
        stationIds[index] = DeserializeStationId(data + offset);
        offset += cStationIdSize;
    }

    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job != 0 && !job->m_fielda2 && job->m_processType != 0 &&
        job->m_processType == processType && hostStationId == job->m_leaveStationId &&
        job->m_field60 == messageType)
    {
        job->ReceiveLeaveMeshInvitation(static_cast<Session::ProcessType>(processType),
                                        hostStationId);
        for (unsigned int index = 0; index < stationIdNum; ++index)
            g_piaSession->m_stationIdStatusTable->SetPrepareStatus(stationIds[index], true);
        SendAnswerToInvitationCore(cMessageTypeAnswerToLeaveMeshInvitation, processType,
                                   hostStationId, true);
    }
}

void SessionProtocol::ParseAnswerToInvitation(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char answer = data[cAnswerResultOffset];
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long stationId = DeserializeStationId(data + cAnswerStationIdOffset);

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return;
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job == 0)
        return;
    if (job->m_processType != 0 && job->m_processType == processType &&
        job->m_field60 == messageType)
        g_piaSession->m_stationIdStatusTable->SetAnswer(stationId, answer != 0);
}

void SessionProtocol::ParseCompletionForMigrateSession(
    const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    bool completed = data[cCompletionMigrateResultOffset] != 0;
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long stationId = DeserializeStationId(data + cCompletionMigrateStationIdOffset);

    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return;

    Session* session = g_piaSession;
    session->m_stationIdStatusTable->SetMigrationCompleteStatus(stationId, completed);
    JointSessionJob* job = session->m_jointSessionJob;
    if (job == 0 || job->m_fielda2)
        return;

    if (job->m_processType == 0)
    {
        if (session->GetStatus() == Session::cConnectionStatusDisconnectedNetwork ||
            session->GetStatus() == Session::cConnectionStatusConnected)
            SendCompletionInvitation(processType, &stationId, 1, session->m_localStationId, 0, 0);
        else
            SendCompletionFailureResponse(processType, stationId);
    }
    else if (job->m_fielda0 != 0)
    {
        SendCompletionInvitation(processType, &stationId, 1, g_piaSession->m_localStationId, 0,
                                 0);
    }
}

void SessionProtocol::ParseCompletionFailureResponse(
    const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    unsigned long long stationId =
        DeserializeStationId(message.m_message + cCompletionFailureStationIdOffset);
    transport::StationIdTable::Entry entry;
    Result result = transport::g_piaTransport->m_stationIdTable->FindByStationId(&entry, stationId);
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (result == 0 && job != 0 && !job->m_fielda2 && job->m_processType != 0)
    {
        job->m_fielda2 = 1;
        job->m_fielda4 = cResultSessionProcessFailed;
    }
}

void SessionProtocol::ParseCompletionInvitation(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned char messageType = data[cMessageTypeOffset];
    unsigned char processType = data[cProcessTypeOffset];
    unsigned long long hostStationId = DeserializeStationId(data + cInvitationStationIdOffset);
    JointSessionJob* job = g_piaSession->m_jointSessionJob;
    if (job != 0 && !job->m_fielda2)
    {
        if (job->m_processType != 0 && job->m_processType == processType &&
            hostStationId == job->m_leaveStationId && job->m_field60 == messageType)
            job->ReceiveCompletionResponse(static_cast<Session::ProcessType>(processType),
                                           hostStationId);
        SendAnswerToCompletionInvitation(processType, hostStationId, true);
    }
}

void SessionProtocol::ParseHelper(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_messageSize == 0)
        return;

    switch (message.m_message[cMessageTypeOffset])
    {
    case cMessageTypeNextSessionId:
        ParseNextSessionId(message);
        break;
    case cMessageTypePreparedForMigrateSession:
        ParsePreparedForMigrateSession(message);
        break;
    case cMessageTypeInvitationAsCompanion:
        ParseInvitationAsCompanion(message);
        break;
    case cMessageTypeAnswerToInvitation:
    case cMessageTypeAnswerToDestroyInvitation:
    case cMessageTypeAnswerToLeaveMeshInvitation:
    case cMessageTypeAnswerToCompletionInvitation:
        ParseAnswerToInvitation(message);
        break;
    case cMessageTypeDestroyInvitation:
        ParseDestroyInvitation(message);
        break;
    case cMessageTypeLeaveMeshInvitation:
        ParseLeaveMeshInvitation(message);
        break;
    case cMessageTypeCompletionForMigrateSession:
        ParseCompletionForMigrateSession(message);
        break;
    case cMessageTypeCompletionInvitation:
        ParseCompletionInvitation(message);
        break;
    case cMessageTypeCompletionFailureResponse:
        ParseCompletionFailureResponse(message);
        break;
    default:
        break;
    }
}

Result SessionProtocol::UpdateProtocolEvent(const transport::ProtocolEvent& event)
{
    transport::Station* station = transport::g_piaStationManager->GetStation(event.m_stationAddress);
    int stationIndex = static_cast<int>(station->m_field10);
    if (m_stationIndex > cStationIndexMax)
        return cResultInvalidState;
    if (stationIndex < 0 || stationIndex >= static_cast<int>(m_windowNum) + 1)
        return cResultInvalidArgument;

    transport::ReliableSlidingWindow* window;
    if (stationIndex < m_stationIndex)
        window = m_windows + stationIndex;
    else if (m_stationIndex < stationIndex)
        window = m_windows + (stationIndex - 1);
    else
        return cResultInvalidArgument;

    if (event.m_type == 0)
    {
        unsigned int protocolType = GetProtocolType();
        Result result = window->Startup(handlerOf(GetStationPacketHandler()),
                                        ((protocolType & 0xff) << 8) | 1, m_stationId,
                                        (static_cast<unsigned long long>(station->m_field18) << 32) |
                                            station->m_field1c);
        return result;
    }
    if (event.m_type == 1)
    {
        window->Cleanup();
        return 0;
    }
    return cResultInvalidArgument;
}

Result SessionProtocol::Dispatch()
{
    if (m_stationIndex == cStationIndexNone)
        return 0;

    transport::ReceivedMessageAccessor message;
    transport::StationPacketHandler* handler = handlerOf(mStationPacketHandler);
    transport::ProtocolId protocolId(
        static_cast<unsigned short>(((GetProtocolType() & 0xff) << 8) | 1));
    transport::PacketHandler::Iterator& iterator = handler->GetIterator(protocolId);
    iterator.BeginIteration();
    for (; !iterator.IsEndIteration(); iterator.NextIteration())
    {
        transport::ProtocolMessageReader* reader = iterator.GetMessageReader();
        transport::Station* station =
            transport::g_piaStationManager->GetStationByStationIdWithoutConvert(
                reader->m_header.m_source);
        if (station == 0 || static_cast<int>(station->m_field10) > cStationIndexMax)
            continue;
        int stationIndex = static_cast<int>(station->m_field10);
        transport::ReliableSlidingWindow* window;
        if (stationIndex < m_stationIndex)
            window = m_windows + stationIndex;
        else if (m_stationIndex < stationIndex)
            window = m_windows + (stationIndex - 1);
        else
            continue;
        if (window->IsInCommunication())
            window->AnalyzeProtocolMessage(*reader);
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        transport::ReliableSlidingWindow* window = m_windows + index;
        if (!window->IsInCommunication())
            continue;
        transport::Station* station =
            transport::g_piaStationManager->GetStationByStationIdWithoutConvert(
                window->m_remoteStationId);
        if (station == 0 || station->m_field20 != cStationStateConnected)
            continue;
        unsigned int size;
        for (;;)
        {
            Result popResult = window->PopData(m_messageBuffer, &size, sizeof(m_messageBuffer));
            if (popResult != 0)
                break;
            message.m_message = m_messageBuffer;
            message.m_messageSize = size;
            message.m_sourceStationId =
                (static_cast<unsigned long long>(station->m_field18) << 32) | station->m_field1c;
            common::StationAddress stationAddress(station->m_stationAddress);
            message.m_stationAddress = stationAddress;
            message.m_field1c = 0;
            ParseHelper(message);
        }
    }

    ++m_dispatchWindowIndex;
    if (m_dispatchWindowIndex >= m_windowNum)
        m_dispatchWindowIndex = 0;
    for (unsigned short index = m_dispatchWindowIndex; index < m_windowNum; ++index)
    {
        if (m_windows[index].IsInCommunication())
            m_windows[index].Dispatch(handlerOf(GetStationPacketHandler()), 0, cUnlimitedDispatchSize);
    }
    for (unsigned short index = 0; index < m_dispatchWindowIndex; ++index)
    {
        if (m_windows[index].IsInCommunication())
            m_windows[index].Dispatch(handlerOf(GetStationPacketHandler()), 0, cUnlimitedDispatchSize);
    }
    return 0;
}

}}}
