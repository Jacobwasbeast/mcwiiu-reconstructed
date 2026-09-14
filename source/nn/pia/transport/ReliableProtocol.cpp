#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>

namespace nn { namespace pia { namespace transport {

common::WatermarkManager* g_piaWatermarkManager = 0;

static int g_reliableProtocolSendFailCount = 0;
static unsigned int g_reliableProtocolResendParam1 = 0;
static unsigned int g_reliableProtocolResendParam2 = 0;

ReliableProtocol::ReliableProtocol()
    : Protocol(),
      m_field14(0xfd),
      m_field18(0),
      m_field1c(0),
      m_field20(0),
      m_field24(0),
      m_field28(0),
      m_field2c(0),
      m_field30(0)
{
    if (g_piaWatermarkManager != 0)
    {
        g_piaWatermarkManager->GetWatermark(3)->SetName("ReliableProtocol send buffer num");
        if (g_piaWatermarkManager != 0)
        {
            g_piaWatermarkManager->GetWatermark(4)->SetName("ReliableProtocol receive buffer num");
        }
    }
}

ReliableProtocol::~ReliableProtocol()
{
    Finalize();
}

void ReliableProtocol::Trace(unsigned long long) const
{
}

unsigned short ReliableProtocol::GetProtocolType() const
{
    return cProtocolType;
}

void ReliableProtocol::Finalize()
{
    m_field14 = 0xfd;
    m_field1c = 0;
    m_field20 = 0;
    m_field2c = 0;
    unsigned char* array = m_field24;
    m_field30 = 0;
    m_field18 = 0;
    if (array != 0)
    {
        unsigned int count = pead::getArraySize(array) >> 7;
        unsigned char* p = array;
        for (; count != 0; --count)
        {
            reinterpret_cast<ReliableSlidingWindow*>(p)->~ReliableSlidingWindow();
            p = p + 0x80;
        }
        pead::freeToSeadHeap(array);
        m_field24 = 0;
    }
}

Result ReliableProtocol::Initialize(unsigned int a1, unsigned int a2)
{
    if (m_field24 != 0)
        return 0x10c02;

    m_field14 = 0xfd;
    m_field18 = 0;
    m_field1c = 0;
    unsigned int stationNum = g_piaTransport->m_stationNum;
    m_field20 = stationNum;
    int count = static_cast<int>(stationNum) - 1;

    pead::Heap* heap = common::HeapManager::GetHeap();
    unsigned char* array = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(static_cast<unsigned int>(count) * 0x80, heap));
    if (array == 0)
    {
        m_field24 = 0;
    }
    else
    {
        unsigned char* p = array;
        for (int i = count; i != 0; --i)
        {
            new (p) ReliableSlidingWindow();
            p = p + 0x80;
        }
        m_field24 = array;
    }

    if (count != 0)
    {
        int offset = 0;
        for (;;)
        {
            Result r = reinterpret_cast<ReliableSlidingWindow*>(array + offset)->Initialize(a1, a2);
            if (r != 0)
            {
                Finalize();
                return r;
            }
            --count;
            offset += 0x80;
            if (count == 0)
                break;
            array = m_field24;
        }
    }

    m_field30 = 0;
    m_field2c = 0x0fffffff;
    g_reliableProtocolResendParam1 = a1;
    g_reliableProtocolResendParam2 = a2;
    return 0;
}

Result ReliableProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    if (m_field24 == 0)
        return 0x10c0c;
    if (m_field14 != 0xfd)
        return 0x10c08;

    void* station = g_piaStationManager->GetStation(address);
    if (station == 0)
        return 0x10c07;

    const unsigned char* raw = reinterpret_cast<const unsigned char*>(station);
    int stationIndex = *reinterpret_cast<const int*>(raw + 0x10);
    if (stationIndex < 0 || m_field20 <= static_cast<unsigned int>(stationIndex))
        return 0x10c07;

    m_field14 = stationIndex;
    unsigned int hi = *reinterpret_cast<const unsigned int*>(raw + 0x18);
    unsigned int lo = *reinterpret_cast<const unsigned int*>(raw + 0x1c);
    m_field30 = 0;
    m_field1c = lo;
    m_field28 = 0;
    m_field18 = hi;
    g_reliableProtocolSendFailCount = 0;
    return 0;
}

void ReliableProtocol::CleanupWithStationAddress()
{
    if (m_field14 != 0xfd)
    {
        m_field30 = 0;
        int count = static_cast<int>(m_field20) - 1;
        m_field14 = 0xfd;
        m_field1c = 0;
        m_field18 = 0;
        if (count != 0)
        {
            int offset = 0;
            do
            {
                ReliableSlidingWindow* window =
                    reinterpret_cast<ReliableSlidingWindow*>(m_field24 + offset);
                if (window->IsInCommunication())
                    window->Cleanup();
                --count;
                offset += 0x80;
            } while (count != 0);
        }
    }
}

Result ReliableProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    if (m_field14 == 0xfd)
        return 0x10c08;

    const unsigned char* eventRaw = reinterpret_cast<const unsigned char*>(&event);
    const common::StationAddress& address =
        *reinterpret_cast<const common::StationAddress*>(eventRaw + 4);
    void* station = g_piaStationManager->GetStation(address);
    const unsigned char* stationRaw = reinterpret_cast<const unsigned char*>(station);
    int stationIndex = *reinterpret_cast<const int*>(stationRaw + 0x10);

    if (stationIndex < 0 || m_field20 <= static_cast<unsigned int>(stationIndex))
        return 0x10c07;
    if (stationIndex == m_field14)
        return 0x10c07;

    ReliableSlidingWindow* window = GetSlidingWindow(static_cast<StationIndex>(stationIndex));
    int eventType = *reinterpret_cast<const int*>(eventRaw);
    if (eventType == 0)
    {
        unsigned short protocolIdValue = mProtocolId;
        unsigned int remoteHi = *reinterpret_cast<const unsigned int*>(stationRaw + 0x18);
        unsigned int remoteLo = *reinterpret_cast<const unsigned int*>(stationRaw + 0x1c);
        unsigned long long localStationId =
            (static_cast<unsigned long long>(m_field18) << 32) | m_field1c;
        unsigned long long remoteStationId =
            (static_cast<unsigned long long>(remoteHi) << 32) | remoteLo;
        return window->Startup(reinterpret_cast<StationPacketHandler*>(GetStationPacketHandler()),
                                protocolIdValue, localStationId, remoteStationId);
    }
    if (eventType == 1)
    {
        window->Cleanup();
        return 0;
    }
    return 0x10c07;
}

ReliableSlidingWindow* ReliableProtocol::GetSlidingWindow(StationIndex stationIndex)
{
    int index = stationIndex;
    if (m_field14 <= index)
        index = index - 1;
    return reinterpret_cast<ReliableSlidingWindow*>(m_field24 + index * 0x80);
}

const ReliableSlidingWindow* ReliableProtocol::GetSlidingWindow(StationIndex stationIndex) const
{
    int index = stationIndex;
    if (m_field14 <= index)
        index = index - 1;
    return reinterpret_cast<const ReliableSlidingWindow*>(m_field24 + index * 0x80);
}

Result ReliableProtocol::Send(StationIndex stationIndex, const void* data, unsigned int size)
{
    unsigned char* array = m_field24;
    if (array == 0)
        return 0x10c08;
    if (m_field14 == 0xfd)
        return 0x2c26;
    if (stationIndex == m_field14)
        return 0x10c07;

    if (stationIndex == static_cast<StationIndex>(0xff))
    {
        int count = static_cast<int>(m_field20) - 1;
        if (count != 0)
        {
            int offset = 0;
            int iterationsLeft = count;
            for (;;)
            {
                ReliableSlidingWindow* window =
                    reinterpret_cast<ReliableSlidingWindow*>(array + offset);
                if (window->IsInCommunication() && window->CanPushData(size) == 0)
                {
                    if (g_reliableProtocolSendFailCount != -1)
                        ++g_reliableProtocolSendFailCount;
                    return 0x4c0d;
                }
                --iterationsLeft;
                offset += 0x80;
                if (iterationsLeft == 0)
                    break;
                array = m_field24;
            }
        }

        if (count != 0)
        {
            int offset = 0;
            do
            {
                ReliableSlidingWindow* window =
                    reinterpret_cast<ReliableSlidingWindow*>(m_field24 + offset);
                if (window->IsInCommunication())
                {
                    Result r = window->PushData(data, size);
                    if (r != 0)
                        return r;
                    if (g_piaWatermarkManager != 0)
                    {
                        unsigned int trackedValue = *reinterpret_cast<const unsigned int*>(
                            reinterpret_cast<const unsigned char*>(window) + 0x28);
                        g_piaWatermarkManager->GetWatermark(3)->Update(trackedValue);
                    }
                }
                --count;
                offset += 0x80;
            } while (count != 0);
        }
        return 0;
    }

    if (stationIndex < 0 || m_field20 <= static_cast<unsigned int>(stationIndex))
        return 0x10c07;

    ReliableSlidingWindow* window = GetSlidingWindow(stationIndex);
    if (!window->IsInCommunication())
        return 0x2c26;

    Result r = window->PushData(data, size);
    if (r == 0)
    {
        if (g_piaWatermarkManager != 0)
        {
            unsigned int trackedValue = *reinterpret_cast<const unsigned int*>(
                reinterpret_cast<const unsigned char*>(window) + 0x28);
            g_piaWatermarkManager->GetWatermark(3)->Update(trackedValue);
        }
    }
    else if (r == 0x4c0d && g_reliableProtocolSendFailCount != -1)
    {
        ++g_reliableProtocolSendFailCount;
    }
    return r;
}

Result ReliableProtocol::Send(unsigned long long stationId, const void* data, unsigned int size)
{
    StationIndex index = 0;
    Result r = g_piaTransport->ConvertToStationIndex(&index, stationId);
    if (r == 0)
        return Send(index, data, size);
    return r;
}

void ReliableProtocol::SendToAll(const void* data, unsigned int size)
{
    Send(static_cast<StationIndex>(0xff), data, size);
}

Result ReliableProtocol::Receive(unsigned long long* stationId, void* buffer, unsigned int* size,
                                  unsigned int bufferSize)
{
    if (stationId == 0)
        return 0x10c07;
    if (m_field14 == 0xfd)
        return 0x2c26;

    int count = static_cast<int>(m_field20) - 1;
    if (count != 0)
    {
        int offset = 0;
        do
        {
            ReliableSlidingWindow* window =
                reinterpret_cast<ReliableSlidingWindow*>(m_field24 + offset);
            if (window->IsInCommunication())
            {
                const unsigned char* raw = reinterpret_cast<const unsigned char*>(window);
                unsigned long long windowStationId =
                    (static_cast<unsigned long long>(*reinterpret_cast<const unsigned int*>(raw + 0x10)) << 32) |
                    *reinterpret_cast<const unsigned int*>(raw + 0x14);
                if (g_piaTransport->m_stationIdTable->IsJoinEventNotifiedByStationId(windowStationId))
                {
                    Result popResult = window->PopData(buffer, size, bufferSize);
                    if (popResult == 0)
                    {
                        if (g_piaWatermarkManager != 0)
                        {
                            unsigned int trackedValue =
                                *reinterpret_cast<const unsigned int*>(raw + 0x3c);
                            g_piaWatermarkManager->GetWatermark(4)->Update(trackedValue);
                        }
                        *stationId = windowStationId;
                        return 0;
                    }
                    if (popResult != 0x2c09)
                        return popResult;
                }
            }
            --count;
            offset += 0x80;
        } while (count != 0);
    }
    return 0x2c09;
}

bool ReliableProtocol::IsInCommunication(StationIndex stationIndex) const
{
    if (m_field14 != 0xfd && stationIndex >= 0 &&
        stationIndex < static_cast<int>(m_field20) && stationIndex != m_field14)
    {
        return GetSlidingWindow(stationIndex)->IsInCommunication();
    }
    return false;
}

bool ReliableProtocol::IsInCommunication(unsigned long long stationId) const
{
    return IsInCommunication(Conv2StationIndex(stationId));
}

}}}
