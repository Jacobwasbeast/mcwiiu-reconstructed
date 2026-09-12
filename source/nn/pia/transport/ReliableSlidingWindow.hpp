#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELIABLESLIDINGWINDOW_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace transport {

class StationPacketHandler;

class ReliableSlidingWindow
{
public:
    ReliableSlidingWindow();
    ~ReliableSlidingWindow();
    Result Initialize(unsigned int a1, unsigned int a2);
    void Cleanup();
    bool IsInCommunication() const;
    Result PopData(void* buffer, unsigned int* size, unsigned int bufferSize);
    Result PushData(const void* data, unsigned int size);
    unsigned int CanPushData(unsigned int size);
    Result Startup(StationPacketHandler* handler, unsigned short protocolId,
                   unsigned long long localStationId, unsigned long long remoteStationId);

private:
    unsigned short m_field00;
    unsigned char m_pad02[6];
    unsigned char m_field08[8];
    unsigned long long m_stationId;
    unsigned char m_pad18[0x10];
    unsigned int m_field28;
    unsigned char m_pad2c[0x10];
    unsigned int m_field3c;
    unsigned int m_field40;
    unsigned char m_pad44[0x3c];
};

extern unsigned short g_reliableSlidingWindowGeneration;

}}}

#endif
