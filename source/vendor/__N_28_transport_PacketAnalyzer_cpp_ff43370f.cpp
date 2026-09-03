#include "vendor/__N_28_transport_PacketAnalyzer_cpp_ff43370f.hpp"

static uint64_t* s_packetAnalyzerTime = 0;

uint64_t __N_28_transport_PacketAnalyzer_cpp_ff43370f::getTime()
{
    if (s_packetAnalyzerTime != 0)
    {
        return *s_packetAnalyzerTime;
    }
    return 0;
}
