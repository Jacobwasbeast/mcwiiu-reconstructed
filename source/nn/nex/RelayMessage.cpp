#include "nn/nex/RelayMessage.hpp"

#include <string.h>

#include <algorithm>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/MD5.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/SignatureBytes.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned char RELAY_PROTOCOL_BYTE = 0xF5;
const unsigned char RELAY_OUTGOING_BYTE = 0xD0;
const unsigned char RELAY_DEFAULT_PORT = 0xBF;
const unsigned int RELAY_TYPE_MASK = 0xF;
const unsigned int RELAY_HEADER_LENGTH = 0x1C;
const unsigned int RELAY_PAYLOAD_OFFSET = 0x2C;
const unsigned int RELAY_PING_HEADER_LENGTH = 0xC;
const int RELAY_PING_PADDING = 2;
const unsigned int RELAY_SIGNATURE_LENGTH = 16;
const unsigned int RELAY_GATHERING_ID_KEY_OFFSET = 4;
const unsigned int RELAY_SIGNED_DATA_LIMIT = 50;
const unsigned int RELAY_GAME_SERVER_ID_MASK = 0xDB319579u;
const unsigned int RELAY_GATHERING_ID_MASK = 0xBEE17D2Fu;
const int RELAY_TIME_STAMP_TOLERANCE = 150;
const unsigned int RELAY_TIME_STAMP_UNIT = 1000;
const unsigned int RELAY_GAME_SERVER_ID_PING = 0xFFFFFFFEu;
const unsigned int RELAY_GAME_SERVER_ID_BROADCAST = 0xFFFFFFFDu;

inline void AppendByte(Buffer* pBuffer, unsigned char uiValue)
{
    pBuffer->AppendData(&uiValue, sizeof(uiValue), APPEND_AT_END);
}

inline void AppendShort(Buffer* pBuffer, unsigned short uiValue)
{
    unsigned char aBytes[2];

    aBytes[0] = static_cast<unsigned char>(uiValue);
    aBytes[1] = static_cast<unsigned char>(uiValue >> 8);
    pBuffer->AppendData(aBytes, sizeof(aBytes), APPEND_AT_END);
}

inline void AppendInt(Buffer* pBuffer, unsigned int uiValue)
{
    unsigned char aBytes[4];

    aBytes[0] = static_cast<unsigned char>(uiValue);
    aBytes[1] = static_cast<unsigned char>(uiValue >> 8);
    aBytes[2] = static_cast<unsigned char>(uiValue >> 16);
    aBytes[3] = static_cast<unsigned char>(uiValue >> 24);
    pBuffer->AppendData(aBytes, sizeof(aBytes), APPEND_AT_END);
}

}

RelayMessage::RelayMessage()
{
}

RelayMessage::~RelayMessage()
{
}

void RelayMessage::SetRelayDataPacketSignatureKey(const unsigned char* pKey, unsigned int uiLength)
{
    m_signatureKey.Clear();
    m_signatureKey.AppendData(pKey, uiLength, APPEND_AT_END);
}

void RelayMessage::SetRelayDataPacketSignatureKeyFromP2PDataPacketSessionSignatureKey(
    const qVector<unsigned char>& rKey)
{
    const unsigned char* pKey = &rKey[0];
    const unsigned int uiGatheringID = static_cast<unsigned int>(pKey[0])
        | static_cast<unsigned int>(pKey[1]) << 8 | static_cast<unsigned int>(pKey[2]) << 16
        | static_cast<unsigned int>(pKey[3]) << 24;

    RootTransport::GetInstance()->GetRelay()->SetRelayGatheringID(uiGatheringID);
    SetRelayDataPacketSignatureKey(&rKey[RELAY_GATHERING_ID_KEY_OFFSET], RELAY_SIGNATURE_LENGTH);
}

void RelayMessage::CalcSignature(SignatureBytes* pSignature, RelayType type,
    const unsigned char* pKey, unsigned int uiKeyLength, const unsigned char* pHeader,
    const unsigned char* pPayload, unsigned int uiPayloadLength)
{
    MD5 md5;

    md5.update(pKey, uiKeyLength);
    md5.update(pHeader, RELAY_HEADER_LENGTH);

    if (pPayload != 0 && uiPayloadLength != 0)
    {
        unsigned int uiSignedLength = uiPayloadLength;

        if (type == RELAY_TYPE_DATA)
        {
            uiSignedLength = std::min(uiPayloadLength, RELAY_SIGNED_DATA_LIMIT);
        }

        md5.update(pPayload, uiSignedLength);
    }

    md5.finalize();
    memcpy(pSignature->m_bytes, md5.GetDigest(), RELAY_SIGNATURE_LENGTH);
    pSignature->m_uiLength = RELAY_SIGNATURE_LENGTH;
}

int RelayMessage::GetCurrentTimeStamp()
{
    int iTimeStamp = m_timeStampOrigin.m_uiBaseTimeStamp;
    const long long iElapsed = static_cast<long long>(g_lastTime - m_timeStampOrigin.m_uiStartTime);

    iTimeStamp += static_cast<int>(iElapsed / RELAY_TIME_STAMP_UNIT);
    return iTimeStamp;
}

bool RelayMessage::CheckTimeStamp(PacketIn* pPacket)
{
    if (pPacket->GetRelayHeader().m_uiGameServerID != RELAY_GAME_SERVER_ID_PING
        && pPacket->GetRelayHeader().m_uiGameServerID != RELAY_GAME_SERVER_ID_BROADCAST)
    {
        const int iNow = GetCurrentTimeStamp();
        const int iTimeStamp = pPacket->GetRelayHeader().m_uiTimeStamp;

        const int iEarliest = iNow - RELAY_TIME_STAMP_TOLERANCE;
        const int iLatest = iNow + RELAY_TIME_STAMP_TOLERANCE;

        bool bInRange = false;

        if (iTimeStamp == iEarliest || iTimeStamp - iEarliest > 0)
        {
            if (iLatest == iTimeStamp || iLatest - iTimeStamp > 0)
            {
                bInRange = true;
            }
        }

        if (!bInRange)
        {
            return false;
        }
    }

    return true;
}

bool RelayMessage::CheckSignature(ByteStream* pStream, PacketIn* pPacket,
    const unsigned char* pKey, unsigned int uiKeyLength)
{
    bool bValid = false;
    SignatureBytes signature;
    signature.m_uiLength = 0;

    Buffer* pBuffer = pStream->GetBuffer();
    const unsigned char* pData = pBuffer->GetData();

    CalcSignature(&signature, static_cast<RelayType>(pPacket->GetRelayHeader().m_uiType & RELAY_TYPE_MASK),
        pKey, uiKeyLength, pData, pData + RELAY_PAYLOAD_OFFSET,
        pBuffer->GetLength() - RELAY_PAYLOAD_OFFSET);

    if (pPacket->m_uiSignatureSize == signature.m_uiLength)
    {
        bValid = memcmp(pPacket->m_aSignature, signature.m_bytes, signature.m_uiLength) == 0;
    }

    return bValid;
}

int RelayMessage::CalcSignature(PacketOut*, SignatureMethod, const Key*, Stream::Type,
    TransportSignatureGenerator*, const SignatureBytes*, SignatureBytes&)
{
    return 0;
}

void RelayMessage::CalcSignature(Packet*, SignatureMethod, const Key*, Stream::Type,
    TransportSignatureGenerator*, const SignatureBytes*, SignatureBytes&)
{
}

RelayMessage::SignatureMethod RelayMessage::DecideSignatureMethod(const Packet*, Stream::Type,
    bool)
{
    return SIGNATURE_METHOD_0;
}

int RelayMessage::CalcExpectedSignature(const Packet*, SignatureMethod, const Key*,
    TransportSignatureGenerator*, const SignatureBytes*, SignatureBytes&)
{
    return 3;
}

bool RelayMessage::FastUnpack(ByteStream*, PacketIn*, unsigned int*)
{
    return false;
}

bool RelayMessage::UnpackPayload(const ByteStream* pStream, PacketIn* pPacket)
{
    Buffer* pPayload = pPacket->m_pPayload;
    Buffer* pBuffer = pStream->GetBuffer();

    pPayload->Clear();

    const unsigned short uiLength =
        static_cast<unsigned short>(pBuffer->GetLength() - RELAY_PAYLOAD_OFFSET);

    return uiLength == 0
        || pPayload->AppendData(pBuffer->GetData() + pStream->GetPosition(), uiLength,
               APPEND_AT_END);
}

bool RelayMessage::UnpackWithoutValidation(ByteStream* pStream, PacketIn* pPacket,
    unsigned int* pConsumed, bool bUnpackPayload)
{
    if (pStream->GetBuffer()->GetLength() != 0)
    {
        unsigned char uiProtocol = 0;
        unsigned char uiMarker = 0;
        unsigned short uiType = 0;
        unsigned int uiSourceAddress = 0;
        unsigned int uiDestinationAddress = 0;
        unsigned short uiSourcePort = 0;
        unsigned short uiDestinationPort = 0;
        unsigned int uiGameServerID = 0;
        unsigned int uiGatheringID = 0;
        unsigned int uiTimeStamp = 0;

        if (pStream->ExtractPrimitive(&uiProtocol) && uiProtocol == RELAY_PROTOCOL_BYTE
            && pStream->ExtractPrimitive(&uiMarker))
        {
            pPacket->m_uiVersion = uiMarker;

            if (pStream->ExtractPrimitive(&uiType))
            {
                pPacket->GetRelayHeader().m_uiType = uiType;

                if (pStream->ExtractPrimitive(&uiSourceAddress))
                {
                    pPacket->GetRelayHeader().m_uiSourceAddress = uiSourceAddress;

                    if (pStream->ExtractPrimitive(&uiDestinationAddress))
                    {
                        pPacket->GetRelayHeader().m_uiDestinationAddress = uiDestinationAddress;

                        if (pStream->ExtractPrimitive(&uiSourcePort))
                        {
                            pPacket->GetRelayHeader().m_uiSourcePort = uiSourcePort;

                            if (pStream->ExtractPrimitive(&uiDestinationPort))
                            {
                                pPacket->GetRelayHeader().m_uiDestinationPort = uiDestinationPort;

                                if (pStream->ExtractPrimitive(&uiGameServerID))
                                {
                                    pPacket->GetRelayHeader().m_uiGameServerID =
                                        uiGameServerID ^ RELAY_GAME_SERVER_ID_MASK;

                                    if (pStream->ExtractPrimitive(&uiGatheringID))
                                    {
                                        pPacket->GetRelayHeader().m_uiGatheringID =
                                            uiGatheringID ^ RELAY_GATHERING_ID_MASK;

                                        if (pStream->ExtractPrimitive(&uiTimeStamp))
                                        {
                                            pPacket->GetRelayHeader().m_uiTimeStamp = uiTimeStamp;

                                            if (pStream->Extract(pPacket->m_aSignature, 1,
                                                    RELAY_SIGNATURE_LENGTH)
                                                && (pPacket->m_uiSignatureSize =
                                                       RELAY_SIGNATURE_LENGTH,
                                                    pStream->GetPosition()
                                                        == RELAY_PAYLOAD_OFFSET))
                                            {
                                                Buffer* pBuffer = pStream->GetBuffer();

                                                pPacket->m_pPayload->Clear();

                                                pPacket->m_uiSource = RELAY_DEFAULT_PORT;
                                                pPacket->m_uiDestination = RELAY_DEFAULT_PORT;

                                                const unsigned short uiPayloadLength =
                                                    static_cast<unsigned short>(
                                                        pBuffer->GetLength()
                                                        - RELAY_PAYLOAD_OFFSET);

                                                if (uiPayloadLength != 0 && bUnpackPayload)
                                                {
                                                    if (!UnpackPayload(pStream, pPacket))
                                                    {
                                                        return false;
                                                    }

                                                    const unsigned int uiPosition =
                                                        pStream->GetPosition() + uiPayloadLength;

                                                    if (uiPosition <= pBuffer->GetLength())
                                                    {
                                                        pStream->SetPosition(uiPosition);
                                                    }
                                                }

                                                if (pConsumed != 0)
                                                {
                                                    *pConsumed = pBuffer->GetLength();
                                                }

                                                return true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool RelayMessage::UnpackPing(PacketIn* pPacket)
{
    if (pPacket->m_pPayload->GetLength() < RELAY_PING_HEADER_LENGTH)
    {
        return false;
    }

    ByteStream stream(pPacket->m_pPayload);
    unsigned char uiFirst;
    unsigned char uiSecond;
    unsigned int uiFirstValue;
    unsigned int uiSecondValue;

    if (stream.ExtractPrimitive(&uiFirst))
    {
        pPacket->GetRelayPingHeader().m_unknown0 = uiFirst;

        if (stream.ExtractPrimitive(&uiSecond))
        {
            pPacket->GetRelayPingHeader().m_unknown1 = uiSecond;

            if (stream.GetPosition() + RELAY_PING_PADDING <= stream.GetLength())
            {
                stream.SetPosition(stream.GetPosition() + RELAY_PING_PADDING);

                if (stream.ExtractPrimitive(&uiFirstValue))
                {
                    pPacket->GetRelayPingHeader().m_unknown4 = uiFirstValue;

                    if (stream.ExtractPrimitive(&uiSecondValue))
                    {
                        pPacket->GetRelayPingHeader().m_unknown8 = uiSecondValue;
                        pPacket->m_pPayload->SetHeadShiftSize(RELAY_PING_HEADER_LENGTH);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool RelayMessage::Unpack(ByteStream* pStream, PacketIn* pPacket, const unsigned char* pKey,
    unsigned int uiKeyLength, unsigned int* pConsumed)
{
    if (UnpackWithoutValidation(pStream, pPacket, pConsumed, true)
        && CheckSignature(pStream, pPacket, pKey, uiKeyLength) && CheckTimeStamp(pPacket))
    {
        if ((pPacket->GetRelayHeader().m_uiType & RELAY_TYPE_MASK) == RELAY_TYPE_PING)
        {
            return UnpackPing(pPacket);
        }

        return true;
    }

    return false;
}

bool RelayMessage::Unpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed)
{
    return Unpack(pStream, pPacket, m_signatureKey.GetData(), m_signatureKey.GetLength(),
        pConsumed);
}

bool RelayMessage::PackPing(PacketOut* pPacket)
{
    Buffer* pPayload = pPacket->m_pPayload;

    if (pPayload != 0 && pPayload->GetLength() != 0)
    {
        if (pPayload->GetHeadShiftSize() == RELAY_PING_HEADER_LENGTH)
        {
            pPayload->SetHeadShiftSize(0);
        }
        else
        {
            Buffer* pBuffer = new Buffer(pPayload->GetLength() + RELAY_PING_HEADER_LENGTH, 0);

            pBuffer->SetLength(RELAY_PING_HEADER_LENGTH);
            pBuffer->AppendData(pPayload->GetData(), pPayload->GetLength(), APPEND_AT_END);
            pPacket->SetPayload(pBuffer);
        }
    }
    else
    {
        Buffer* pBuffer = new Buffer(RELAY_PING_HEADER_LENGTH, 0);

        pBuffer->SetLength(RELAY_PING_HEADER_LENGTH);
        pPacket->SetPayload(pBuffer);
    }

    const unsigned int uiLength = pPacket->m_pPayload->GetLength();

    pPacket->m_pPayload->SetLength(0);

    ByteStream stream(pPacket->m_pPayload);

    AppendByte(stream.GetBuffer(), pPacket->GetRelayPingHeader().m_unknown0);
    AppendByte(stream.GetBuffer(), pPacket->GetRelayPingHeader().m_unknown1);

    for (int i = 0; i < RELAY_PING_PADDING; ++i)
    {
        unsigned char uiPadding = 0;
        stream.GetBuffer()->AppendData(&uiPadding, sizeof(uiPadding), APPEND_AT_END);
    }

    AppendInt(stream.GetBuffer(), pPacket->GetRelayPingHeader().m_unknown4);
    AppendInt(stream.GetBuffer(), pPacket->GetRelayPingHeader().m_unknown8);
    pPacket->m_pPayload->SetLength(uiLength);
    return true;
}

bool RelayMessage::Pack(PacketOut* pPacket, ByteStream* pStream, const unsigned char* pKey,
    unsigned int uiKeyLength)
{
    pStream->Clear();
    AppendByte(pStream->GetBuffer(), RELAY_PROTOCOL_BYTE);

    const unsigned char uiMarker = pPacket->m_uiVersion;

    if (uiMarker == RELAY_OUTGOING_BYTE)
    {
        AppendByte(pStream->GetBuffer(), uiMarker);
        AppendShort(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiType);
        AppendInt(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiSourceAddress);
        AppendInt(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiDestinationAddress);
        AppendShort(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiSourcePort);
        AppendShort(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiDestinationPort);
        AppendInt(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiGameServerID ^ RELAY_GAME_SERVER_ID_MASK);
        AppendInt(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiGatheringID ^ RELAY_GATHERING_ID_MASK);
        AppendInt(pStream->GetBuffer(), pPacket->GetRelayHeader().m_uiTimeStamp);

        SignatureBytes signature;
        signature.m_uiLength = 0;

        const RelayType type = static_cast<RelayType>(pPacket->GetRelayHeader().m_uiType & RELAY_TYPE_MASK);
        Buffer* pPayload = pPacket->m_pPayload;
        const unsigned char* pData = pStream->GetBuffer()->GetData();
        bool bAppended;

        if (pPayload != 0)
        {
            CalcSignature(&signature, type, pKey, uiKeyLength, pData, pPayload->GetData(),
                pPayload->GetLength());
            bAppended = pStream->AppendRaw(signature.m_bytes, signature.m_uiLength);
        }
        else
        {
            CalcSignature(&signature, type, pKey, uiKeyLength, pData, 0, 0);
            bAppended = pStream->AppendRaw(signature.m_bytes, signature.m_uiLength);
        }

        if (bAppended)
        {
            if (pPacket->m_pPayload != 0)
            {
                *pStream << *pPacket->m_pPayload;
            }

            return true;
        }
    }

    return false;
}

bool RelayMessage::Pack(PacketOut* pPacket, ByteStream* pStream)
{
    if ((pPacket->GetRelayHeader().m_uiType & RELAY_TYPE_MASK) == RELAY_TYPE_PING && !PackPing(pPacket))
    {
        return false;
    }

    pPacket->GetRelayHeader().m_uiTimeStamp = GetCurrentTimeStamp();
    return Pack(pPacket, pStream, m_signatureKey.GetData(), m_signatureKey.GetLength());
}

}
}
