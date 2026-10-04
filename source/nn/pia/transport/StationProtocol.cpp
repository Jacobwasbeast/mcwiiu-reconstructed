#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace transport {

namespace {

inline void writeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[3] = static_cast<unsigned char>(value);
}

inline void writeU64(unsigned char* buffer, unsigned long long value)
{
    for (unsigned int index = 0; index < 8; ++index)
        buffer[index] = static_cast<unsigned char>((value >> (56 - index * 8)) & 0xff);
}

}

StationProtocol::StationProtocol()
    : m_timeoutMsec(10000)
{
}

StationProtocol::~StationProtocol()
{
}

Result StationProtocol::Initialize()
{
    return 0;
}

Result StationProtocol::StartupWithStationAddress(const common::StationAddress&)
{
    m_field18 = 0;
    return 0;
}

void StationProtocol::CleanupWithStationAddress()
{
    m_field18 = 0;
}

void StationProtocol::SendDenyingConnectionResponse(const common::StationAddress& address,
                                                    unsigned char rejection,
                                                    unsigned long long principalId,
                                                    unsigned int infoField)
{
    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer =
        reinterpret_cast<PacketHandler*>(mStationPacketHandler)->AssignByStationAddress(
            protocolId, address, 17, false, false);
    if (writer)
    {
        writer->WriteU8(cMessageTypeConnectionResponse, 0);
        writer->WriteU8(rejection, 1);
        writer->WriteU8(8, 2);
        writer->WriteU8(0, 3);
        writer->WriteU8(0, 4);
        writer->WriteU64(principalId, 5);
        writer->WriteU32(infoField, 13);
        reinterpret_cast<PacketHandler*>(mStationPacketHandler)->Commit();
    }
}

void StationProtocol::SendAck(unsigned int value, StationIndex stationIndex)
{
    ProtocolId protocolId = mProtocolId;
    StationPacketHandler* handler =
        reinterpret_cast<StationPacketHandler*>(GetStationPacketHandler());
    ProtocolMessageWriter* writer =
        handler->AssignByStationIndex(protocolId, stationIndex, 8, false, false, true);
    if (writer)
    {
        unsigned char message[8] = { cMessageTypeAck };
        common::serializeU32(message + 4, value);
        writer->SetPayload(message);
        reinterpret_cast<PacketHandler*>(mStationPacketHandler)->Commit();
    }
}

void StationProtocol::SendAck(unsigned int value, const common::StationAddress& address)
{
    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer =
        reinterpret_cast<PacketHandler*>(mStationPacketHandler)->AssignByStationAddress(
            protocolId, address, 8, false, false);
    if (writer)
    {
        unsigned char message[8] = { cMessageTypeAck };
        common::serializeU32(message + 4, value);
        writer->SetPayload(message);
        reinterpret_cast<PacketHandler*>(mStationPacketHandler)->Commit();
    }
}

void StationProtocol::ParseConnectionRequest(const ReceivedMessageAccessor& message)
{
    ParseConnectionRequestCommon(message, false);
}

void StationProtocol::ParseConnectionResponse(const ReceivedMessageAccessor& message)
{
    ParseConnectionResponseCommon(message, false);
}

unsigned int StationProtocol::SendSimpleMessage(unsigned char messageType,
                                                const common::StationAddress& address)
{
    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer =
        reinterpret_cast<PacketHandler*>(mStationPacketHandler)->AssignByStationAddress(
            protocolId, address, 1, false, false);
    if (writer)
    {
        writer->WriteU8(messageType, 0);
        Result result = reinterpret_cast<PacketHandler*>(mStationPacketHandler)->Commit();
        if (result == 0)
            return 1;
    }
    return 0;
}

void StationProtocol::SendDisconnectionResponse(const common::StationAddress& address)
{
    SendSimpleMessage(cMessageTypeDisconnectionResponse, address);
}

void StationProtocol::ParseRelayConnectionRequest(const ReceivedMessageAccessor& message)
{
    ParseConnectionRequestCommon(message, true);
}

void StationProtocol::ParseRelayConnectionResponse(const ReceivedMessageAccessor& message)
{
    ParseConnectionResponseCommon(message, true);
}

Result StationProtocol::ParseHelper(const ReceivedMessageAccessor& message)
{
    if (message.m_messageSize == 0)
        return 0x2c03;
    switch (message.m_message[0])
    {
    case cMessageTypeConnectionRequest:
        ParseConnectionRequest(message);
        break;
    case cMessageTypeConnectionResponse:
        ParseConnectionResponse(message);
        break;
    case cMessageTypeDisconnectionRequest:
        ParseDisconnectionRequest(message);
        break;
    case cMessageTypeDisconnectionResponse:
        ParseDisconnectionResponse(message);
        break;
    case cMessageTypeAck:
        ParseAck(message);
        break;
    case cMessageTypeRelayConnectionRequest:
        ParseRelayConnectionRequest(message);
        break;
    case cMessageTypeRelayConnectionResponse:
        ParseRelayConnectionResponse(message);
        break;
    default:
        return 0x2c03;
    }
    return 0;
}

Result StationProtocol::Dispatch()
{
    ReceivedMessageAccessor message;
    ProtocolId protocolId = mProtocolId;
    PacketHandler* handler = reinterpret_cast<PacketHandler*>(mStationPacketHandler);
    PacketHandler::Iterator& iterator = handler->GetIterator(protocolId);
    while (!handler->IsEndIteration())
    {
        ProtocolMessageReader* reader = iterator.GetMessageReader();
        message.m_message = reader->m_message + ProtocolMessageReader::cMessageHeaderSize;
        message.m_messageSize = reader->m_messageSize;
        message.m_sourceStationId = reader->m_header.m_source;
        message.m_stationAddress = reader->m_stationAddress;
        message.m_field1c = reader->m_field38;
        Result result = ParseHelper(message);
        if (result != 0)
            return result;
        handler->NextIteration();
    }
    return 0;
}

unsigned int StationProtocol::SendSimpleMessage(unsigned char messageType,
                                                StationIndex stationIndex, bool relay)
{
    ProtocolId protocolId = mProtocolId;
    StationPacketHandler* handler =
        reinterpret_cast<StationPacketHandler*>(GetStationPacketHandler());
    ProtocolMessageWriter* writer =
        handler->AssignByStationIndex(protocolId, stationIndex, 1, relay, false, true);
    if (writer)
    {
        writer->WriteU8(messageType, 0);
        Result result = reinterpret_cast<PacketHandler*>(mStationPacketHandler)->Commit();
        if (result == 0)
            return 1;
    }
    return 0;
}

void StationProtocol::SendDisconnectionRequest(StationIndex stationIndex, bool relay)
{
    SendSimpleMessage(cMessageTypeDisconnectionRequest, stationIndex, relay);
}

void StationProtocol::SendDisconnectionRequest(const common::StationAddress& address)
{
    SendSimpleMessage(cMessageTypeDisconnectionRequest, address);
}

unsigned int StationProtocol::GetConnectionRequestDataSize() const
{
    return g_stationConnectionInfoTable->GetSerializedSize() + cConnectionRequestHeaderSize;
}

void StationProtocol::MakeConnectionRequestData(unsigned char* buffer, unsigned char nonce,
                                                 unsigned long long principalId,
                                                 unsigned int infoField,
                                                 unsigned char field55, bool relay,
                                                 bool secure) const
{
    buffer[0] = relay ? cMessageTypeRelayConnectionRequest : cMessageTypeConnectionRequest;
    buffer[1] = nonce;
    buffer[2] = 8;
    buffer[3] = secure;
    writeU64(buffer + 4, principalId);
    writeU32(buffer + 12, infoField);
    buffer[16] = field55;
    unsigned int serializedSize;
    g_stationConnectionInfoTable->Serialize(buffer + cConnectionRequestHeaderSize,
                                            &serializedSize,
                                            GetConnectionRequestDataSize() -
                                                cConnectionRequestHeaderSize);
}

void StationProtocol::Trace(unsigned long long) const
{
}

void StationProtocol::Finalize()
{
}

unsigned int StationProtocol::IsProtocolFilteringEnabled() const
{
    return 0;
}

unsigned short StationProtocol::GetProtocolType() const
{
    return 0x14;
}

}}}
