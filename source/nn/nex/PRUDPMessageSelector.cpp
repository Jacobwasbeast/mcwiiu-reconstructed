#include "nn/nex/PRUDPMessageSelector.hpp"

#include "nn/nex/ByteStream.hpp"
#include "nn/nex/PRUDPMessageV0.hpp"
#include "nn/nex/PRUDPMessageV1.hpp"
#include "nn/nex/PacketIn.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/SignatureBytes.hpp"

namespace __N_19_OnlineCore_Blob_cpp_9428d484
{
int PRUDPVersionCheck(const nn::nex::ByteStream* pStream);
bool PRUDPUnpackCheck(const nn::nex::ByteStream* pStream, unsigned int* pConsumed);
unsigned int RelayProtocolCheck(const nn::nex::ByteStream* pStream);
}

namespace nn
{
namespace nex
{

PRUDPMessageSelector::PRUDPMessageSelector() : m_messages(VersionCount)
{
    m_messages[0] = new PRUDPMessageV0();
    m_messages[1] = new PRUDPMessageV1();
    m_pRelayMessage = new RelayMessage();
}

PRUDPMessageSelector::~PRUDPMessageSelector()
{
    for (unsigned int i = 0; i < m_messages.size(); ++i)
        delete m_messages[i];

    delete m_pRelayMessage;
}

int PRUDPMessageSelector::CalcSignature(PacketOut* pPacket, SignatureMethod method,
    const Key* pKey, Stream::Type type, TransportSignatureGenerator* pGenerator,
    const SignatureBytes* pSignature, SignatureBytes& rResult)
{
    return m_messages[pPacket->m_uiVersion]->CalcSignature(
        pPacket, method, pKey, type, pGenerator, pSignature, rResult);
}

void PRUDPMessageSelector::CalcSignature(Packet* pPacket, SignatureMethod method,
    const Key* pKey, Stream::Type type, TransportSignatureGenerator* pGenerator,
    const SignatureBytes* pSignature, SignatureBytes& rResult)
{
    m_messages[pPacket->m_uiVersion]->CalcSignature(
        pPacket, method, pKey, type, pGenerator, pSignature, rResult);
}

PRUDPMessageInterface::SignatureMethod PRUDPMessageSelector::DecideSignatureMethod(
    const Packet* pPacket, Stream::Type type, bool bFlag)
{
    return m_messages[pPacket->m_uiVersion]->DecideSignatureMethod(pPacket, type, bFlag);
}

int PRUDPMessageSelector::CalcExpectedSignature(const Packet* pPacket, SignatureMethod method,
    const Key* pKey, TransportSignatureGenerator* pGenerator, const SignatureBytes* pSignature,
    SignatureBytes& rResult)
{
    return m_messages[pPacket->m_uiVersion]->CalcExpectedSignature(
        pPacket, method, pKey, pGenerator, pSignature, rResult);
}

bool PRUDPMessageSelector::Unpack(ByteStream* pStream, PacketIn* pPacket, unsigned int* pConsumed)
{
    if (__N_19_OnlineCore_Blob_cpp_9428d484::PRUDPUnpackCheck(pStream, pConsumed))
    {
        if (__N_19_OnlineCore_Blob_cpp_9428d484::RelayProtocolCheck(pStream))
            return m_pRelayMessage->Unpack(pStream, pPacket, pConsumed);

        int iVersion = __N_19_OnlineCore_Blob_cpp_9428d484::PRUDPVersionCheck(pStream);
        if (iVersion >= 0)
            return m_messages[iVersion]->Unpack(pStream, pPacket, pConsumed);
    }

    return false;
}

bool PRUDPMessageSelector::FastUnpack(ByteStream* pStream, PacketIn* pPacket,
    unsigned int* pConsumed)
{
    if (__N_19_OnlineCore_Blob_cpp_9428d484::PRUDPUnpackCheck(pStream, pConsumed))
    {
        if (__N_19_OnlineCore_Blob_cpp_9428d484::RelayProtocolCheck(pStream))
            return m_pRelayMessage->FastUnpack(pStream, pPacket, pConsumed);

        int iVersion = __N_19_OnlineCore_Blob_cpp_9428d484::PRUDPVersionCheck(pStream);
        if (iVersion >= 0)
            return m_messages[iVersion]->FastUnpack(pStream, pPacket, pConsumed);
    }

    return false;
}

bool PRUDPMessageSelector::Pack(PacketOut* pPacket, ByteStream* pStream)
{
    if (pPacket->m_uiSource >> 4 == Stream::Relay)
        return m_pRelayMessage->Pack(pPacket, pStream);

    return m_messages[pPacket->m_uiVersion]->Pack(pPacket, pStream);
}

}
}

namespace __N_19_OnlineCore_Blob_cpp_9428d484
{

namespace
{

const unsigned int PRUDP_MIN_LENGTH = 10;
const unsigned char PRUDP_MAGIC_FIRST = 0xEA;
const unsigned char PRUDP_MAGIC_SECOND = 0xD0;
const unsigned char RELAY_MAGIC_FIRST = 0xF5;
const unsigned char RELAY_MAGIC_SECOND = 0xD0;
const int PRUDP_MAX_VERSION = 1;
const int PRUDP_VERSION_ZERO_MAGIC = -256;

}

int PRUDPVersionCheck(const nn::nex::ByteStream* pStream)
{
    const nn::nex::Buffer* pBuffer = pStream->GetBuffer();
    if (pBuffer->GetData()[0] != PRUDP_MAGIC_FIRST && pBuffer->GetData()[1] != PRUDP_MAGIC_SECOND)
        return 0;

    int iVersion = pBuffer->GetData()[2];
    if (iVersion == 0)
        return PRUDP_VERSION_ZERO_MAGIC;

    if (iVersion > PRUDP_MAX_VERSION)
        return -iVersion;

    return iVersion;
}

bool PRUDPUnpackCheck(const nn::nex::ByteStream* pStream, unsigned int* pConsumed)
{
    if (pConsumed != 0)
        *pConsumed = 0;

    return pStream->GetBuffer()->GetLength() >= PRUDP_MIN_LENGTH;
}

unsigned int RelayProtocolCheck(const nn::nex::ByteStream* pStream)
{
    const nn::nex::Buffer* pBuffer = pStream->GetBuffer();
    if (pBuffer->GetData()[0] == RELAY_MAGIC_FIRST && pBuffer->GetData()[1] == RELAY_MAGIC_SECOND)
        return 1;

    return 0;
}

}
