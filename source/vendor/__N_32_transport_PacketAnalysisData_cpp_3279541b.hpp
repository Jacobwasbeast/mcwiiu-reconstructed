#ifndef SOURCE_VENDOR___N_32_TRANSPORT_PACKETANALYSISDATA_CPP_3279541B_HPP
#define SOURCE_VENDOR___N_32_TRANSPORT_PACKETANALYSISDATA_CPP_3279541B_HPP

#include <stdint.h>

#include "nn/pia/Reconstruction.hpp"

namespace __N_32_transport_PacketAnalysisData_cpp_3279541b
{
    bool isInternalProtocol(nn::pia::transport::ProtocolId protocolId);
    const char *getProtocolName(nn::pia::transport::ProtocolId protocolId);
}

#endif
