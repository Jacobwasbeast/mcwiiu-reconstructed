#include "nn/pia/transport/PacketHandler.hpp"

#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/transport/PacketAnalyzer.hpp"

namespace nn { namespace pia { namespace transport {

Result PacketHandler::Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                              common::IPacketInput* packetInput,
                              common::IPacketOutput* packetOutput,
                              unsigned long long* stationId,
                              const common::CryptoSetting* cryptoSetting)
{
    const unsigned int packetSize =
        (common::g_payloadSizeManager->m_mtuSize & ~3u) -
        common::g_payloadSizeManager->m_signatureSize;

    if (writer == 0 || reader == 0 || packetInput == 0 || packetOutput == 0 || stationId == 0)
        return static_cast<Result>(0x10c07);
    if (cryptoSetting != 0 && cryptoSetting->m_field00 > 1)
        return static_cast<Result>(0x10c07);
    if (m_writer != 0 || m_reader != 0)
        return static_cast<Result>(0x10c08);

    m_packetInput = packetInput;
    m_writer = writer;
    m_reader = reader;
    m_packetOutput = packetOutput;
    m_protocolMessageWriter.SetSource(0);
    m_field358 = 0;

    if (packetSize - cPacketHeaderSize > cPayloadSizeMax)
        return static_cast<Result>(0x10c07);

    if (cryptoSetting == 0)
        m_cryptoSetting = common::CryptoSetting();
    else
        m_cryptoSetting = *cryptoSetting;

    if (m_cryptoSetting.m_field00 == 0)
    {
        m_packetSize = packetSize;
    }
    else if (m_cryptoSetting.m_field00 == 1)
    {
        const unsigned int blockSize = common::Crypto::GetBlockSize(1);
        m_packetSize =
            ((packetSize - cPacketHeaderSize) / blockSize) * blockSize + cPacketHeaderSize;
    }
    m_payloadSize = m_packetSize - cPacketHeaderSize;

    m_sendPacketAnalyzer->Startup();
    m_sendPacketAnalyzer->ClearPacketAnalysisData();
    m_sendUnicastPacketAnalyzer->Startup();
    m_sendUnicastPacketAnalyzer->ClearPacketAnalysisData();
    m_sendBroadcastPacketAnalyzer->Startup();
    m_sendBroadcastPacketAnalyzer->ClearPacketAnalysisData();
    m_receivePacketAnalyzer->Startup();
    m_receivePacketAnalyzer->ClearPacketAnalysisData();

    m_stationId = stationId;
    return 0;
}

}}}
