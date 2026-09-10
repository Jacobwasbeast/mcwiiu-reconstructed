#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn { namespace pia { namespace transport {

StationConnectionInfoTable* StationPacketHandler::GetStationConnectionInfoTable()
{
    return g_stationConnectionInfoTable;
}

Result StationPacketHandler::Startup(PacketStream::Writer* writer,
                                     PacketStream::Reader* reader,
                                     common::IPacketInput* packetInput,
                                     common::IPacketOutput* packetOutput,
                                     unsigned long long* stationId,
                                     const common::CryptoSetting* cryptoSetting)
{
    const Result result = PacketHandler::Startup(writer, reader, packetInput, packetOutput,
                                                 stationId, cryptoSetting);
    if (result != 0)
        return result;

    m_localStationIndex = 0xfd;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    m_protocolMessageWriter.SetSource(0);
    m_keepAliveStationBitmap = 0;
    return 0;
}

}}}
