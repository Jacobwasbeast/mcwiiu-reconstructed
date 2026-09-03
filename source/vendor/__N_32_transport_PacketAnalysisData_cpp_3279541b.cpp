#include "vendor/__N_32_transport_PacketAnalysisData_cpp_3279541b.hpp"

bool __N_32_transport_PacketAnalysisData_cpp_3279541b::isInternalProtocol(nn::pia::transport::ProtocolId protocolId)
{
    unsigned short protocolType = (unsigned short)(protocolId >> 8);
    if (protocolType < 0x2c)
    {
        if (protocolType < 0x1c)
            return protocolType == 8 || protocolType == 0x14 || protocolType == 0x18;
        return protocolType == 0x1c || protocolType == 0x24 || protocolType == 0x28;
    }
    if (protocolType < 0x58)
        return protocolType == 0x2c || protocolType == 0x34 || protocolType == 0x44 || protocolType == 0x54;
    return protocolType == 0x58 || protocolType == 0x94 || protocolType == 0xa4 || protocolType == 0xa8;
}

const char *__N_32_transport_PacketAnalysisData_cpp_3279541b::getProtocolName(nn::pia::transport::ProtocolId protocolId)
{
    unsigned short protocolType = (unsigned short)(protocolId >> 8);
    if (protocolType > 99)
    {
        if (protocolType < 0x74)
        {
            if (protocolType == 100) return "Sync";
            if (protocolType == 0x68) return "Unreliable";
            if (protocolType == 0x6c) return "RoundrobinUnreliable";
            return "(KickoutReason_Unknown PROTOCOL NAME)";
        }
        if (protocolType < 0x84)
        {
            if (protocolType == 0x74) return "Clone";
            if (protocolType == 0x78) return "Voice";
            if (protocolType == 0x7c) return "Reliable";
            return "(KickoutReason_Unknown PROTOCOL NAME)";
        }
        if (protocolType == 0x84) return "ReliableBroadcast";
        if (protocolType == 0x94) return "Session";
        if (protocolType == 0xa4) return "Feedback";
        if (protocolType == 0xa8) return "RelayService";
        return "(KickoutReason_Unknown PROTOCOL NAME)";
    }
    if (protocolType < 0x24)
    {
        if (protocolType == 8) return "KeepAlive";
        if (protocolType == 0x14) return "Station";
        if (protocolType == 0x18) return "Mesh";
        if (protocolType == 0x1c) return "SyncClock";
        return "(KickoutReason_Unknown PROTOCOL NAME)";
    }
    if (protocolType > 0x43)
    {
        if (protocolType == 0x44) return "Lan";
        if (protocolType == 0x54) return "BandwidthCheck";
        if (protocolType == 0x58) return "Rtt";
        return "(KickoutReason_Unknown PROTOCOL NAME)";
    }
    if (protocolType == 0x24) return "Local";
    if (protocolType == 0x28) return "Direct";
    if (protocolType == 0x2c) return "Net";
    if (protocolType == 0x34) return "Nat";
    return "(KickoutReason_Unknown PROTOCOL NAME)";
}
