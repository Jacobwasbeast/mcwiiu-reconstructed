#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_ANALYZER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_ANALYZER_HPP

#include "nn/pia.h"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/ConnectionAnalysisData.hpp"
#include "nn/pia/transport/PacketAnalysisData.hpp"

namespace nn { namespace pia { namespace transport {

class TransportAnalyzer
{
public:
    enum { cPacketAnalysisDataNum = 4 };

    TransportAnalyzer();
    ~TransportAnalyzer();
    static Result CreateInstance();
    static void DestroyInstance();
    Result Initialize();
    void Finalize();
    Result Startup();
    void Cleanup();
    Result Update();
    Result UpdateConnectionAnalysisData();
    void SetMonitoringData();
    void SetMarker(const char* marker);
    void RegisterPrintCallback(void (*callback)(const char*));
    void Trace(unsigned long long value) const;

    bool m_startedUp;
    PacketAnalysisData m_packetAnalysisData[cPacketAnalysisDataNum];
    ConnectionAnalysisData m_connectionAnalysisData;
    unsigned int m_field110c;
    common::Time m_startTime;
};

extern TransportAnalyzer* g_piaTransportAnalyzer;

}}}

#endif
