#include "nn/pia/transport/TransportAnalyzer.hpp"

#include <new>
#include <string.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

TransportAnalyzer* g_piaTransportAnalyzer = 0;

namespace
{
void* g_transportAnalyzerStorage = 0;
}

TransportAnalyzer::TransportAnalyzer()
    : m_startedUp(false), m_field110c(0)
{
}

TransportAnalyzer::~TransportAnalyzer()
{
}

Result TransportAnalyzer::CreateInstance()
{
    if (IsInitialized() == 0)
        return static_cast<Result>(0x10c0c);
    if (IsInSetupMode() == 0)
        return static_cast<Result>(0x10c08);
    if (g_piaTransportAnalyzer != 0)
        return static_cast<Result>(0x10c0f);

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_transportAnalyzerStorage =
        pead::allocateFromSeadHeap(sizeof(TransportAnalyzer), heap, 8);
    if (g_transportAnalyzerStorage == 0)
        return static_cast<Result>(0x10c01);

    memset(g_transportAnalyzerStorage, 0, sizeof(TransportAnalyzer));
    g_piaTransportAnalyzer = new (g_transportAnalyzerStorage) TransportAnalyzer();
    return 0;
}

void TransportAnalyzer::DestroyInstance()
{
    if (g_piaTransportAnalyzer != 0)
    {
        g_piaTransportAnalyzer->~TransportAnalyzer();
        g_piaTransportAnalyzer = 0;
    }
    if (g_transportAnalyzerStorage != 0)
    {
        pead::freeToSeadHeap(g_transportAnalyzerStorage);
        g_transportAnalyzerStorage = 0;
    }
}

Result TransportAnalyzer::Initialize()
{
    return 0;
}

void TransportAnalyzer::Finalize()
{
}

Result TransportAnalyzer::Startup()
{
    if (m_startedUp)
        return static_cast<Result>(0x10c08);
    if (g_piaThreadStreamManager == 0)
        return static_cast<Result>(0x10c08);

    m_connectionAnalysisData.Clear();
    m_startTime = g_piaTransport->m_time;
    m_startedUp = true;
    return 0;
}

void TransportAnalyzer::Cleanup()
{
    if (!m_startedUp)
        return;
    m_startedUp = false;
}

}}}
