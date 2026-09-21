#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_EMULATIONDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_EMULATIONDEVICE_HPP

#include <cstdint>
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class EmulationDevice : public RootObject
{
public:
    EmulationDevice();
    virtual ~EmulationDevice();
    virtual void Enable();

    unsigned int GetBandwidth();
    unsigned int GetJitter();
    unsigned int GetLatency();
    void SetPacketDropProbability(float probability);

    void SetLatency(unsigned int latency);
    void SetJitter(unsigned int jitter);
    void SetProtocolHeaderSize(unsigned int size);
    void SetBandwidth(unsigned int bandwidth);
    unsigned int GetProtocolHeaderSize();

private:
    bool m_enabled;
    std::uint32_t m_field4;
    std::uint32_t m_bandwidth;
    std::uint32_t m_latency;
    std::uint32_t m_jitter;
    float m_packetDropProbability;
};

}
}

#endif
