#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_LATENCY_EMULATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_LATENCY_EMULATOR_HPP

#include "nn/pia.h"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Time.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace common {
class IPacketInput
{
public:
    virtual ~IPacketInput() {}
    virtual Result Read(Packet* packet) = 0;
};

class IPacketOutput
{
public:
    virtual ~IPacketOutput() {}
    virtual unsigned int GetMulticastDestinationNum() const { return 0; }
    virtual bool IsBroadcastSupported() const { return false; }
    virtual Result Write(const Packet& packet) = 0;
};
}}}

namespace nn { namespace pia { namespace transport {

class LatencyEmulator
{
public:
    class Entry
    {
    public:
        common::Packet m_packet;
        common::Time m_dispatchTime;
    };

    explicit LatencyEmulator(unsigned int entryNum);
    LatencyEmulator(common::IPacketInput* packetInput, unsigned int entryNum);
    LatencyEmulator(common::IPacketOutput* packetOutput, unsigned int entryNum);
    ~LatencyEmulator();
    void SetInput(common::IPacketInput* packetInput);
    void SetOutput(common::IPacketOutput* packetOutput);
    void SetLatency(int minimumLatency, int maximumLatency);
    void Clear();
    Result Read(common::Packet* packet);
    Result Write(const common::Packet& packet);
    Result Dispatch();
    void Trace(unsigned long long value) const;

    Entry* m_entries;
    int m_entryNum;
    int m_head;
    int m_tail;
    Entry* m_buffer;
    common::IPacketInput* m_packetInput;
    common::IPacketOutput* m_packetOutput;
    int m_minimumLatency;
    int m_maximumLatency;
    mutable common::CriticalSection m_criticalSection;
    pead::Random m_random;
    Entry m_entry;

private:
    bool allocateBuffer(unsigned int entryNum);
    void freeBuffer();
    void init(unsigned int entryNum);
    void getLatency(int* minimumLatency, int* maximumLatency) const;
    int calcLatency();
    Result writeDispatch();
    Result readDispatch();
};

}}}

#endif
