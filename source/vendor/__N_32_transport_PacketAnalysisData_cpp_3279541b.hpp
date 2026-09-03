#ifndef SOURCE_VENDOR___N_32_TRANSPORT_PACKETANALYSISDATA_CPP_3279541B_HPP
#define SOURCE_VENDOR___N_32_TRANSPORT_PACKETANALYSISDATA_CPP_3279541B_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace transport
{
typedef unsigned short ProtocolId;
}
}
}

class __N_32_transport_PacketAnalysisData_cpp_3279541b
{
public:
    static bool isInternalProtocol(nn::pia::transport::ProtocolId protocolId);
    static const char *getProtocolName(nn::pia::transport::ProtocolId protocolId);
};

#endif
