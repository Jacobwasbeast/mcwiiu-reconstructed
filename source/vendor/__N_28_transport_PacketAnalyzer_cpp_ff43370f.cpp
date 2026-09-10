#include "vendor/__N_28_transport_PacketAnalyzer_cpp_ff43370f.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Time.hpp"

uint64_t __N_28_transport_PacketAnalyzer_cpp_ff43370f::getTime()
{
    if (nn::pia::transport::g_piaTransport == 0)
    {
        nn::pia::common::Time time;
        time.SetNow();
        return time.m_tick;
    }
    return nn::pia::transport::g_piaTransport->m_time.m_tick;
}
