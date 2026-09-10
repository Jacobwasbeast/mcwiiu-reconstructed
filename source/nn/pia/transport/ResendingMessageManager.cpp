#include "nn/pia/transport/ResendingMessageManager.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

ResendingMessageManager* g_piaResendingMessageManager = 0;

namespace
{
void* g_resendingMessageManagerStorage = 0;
}

ResendingMessageManager::ResendingMessageManager()
    : m_stationIds(0), m_timeouts(0), m_ackIds(0), m_messages(0), m_messageSizes(0),
      m_field14(0), m_field18(0), m_field1c(0), m_field20(0), m_field2c(0),
      m_entryNum(0)
{
}

ResendingMessageManager::~ResendingMessageManager()
{
}

Result ResendingMessageManager::CreateInstance()
{
    if (g_piaResendingMessageManager != 0)
        return static_cast<Result>(0x10c0f);

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_resendingMessageManagerStorage =
        pead::allocateFromSeadHeap(sizeof(ResendingMessageManager), heap, 8);
    if (g_resendingMessageManagerStorage == 0)
        return static_cast<Result>(0x10c01);

    g_piaResendingMessageManager =
        new (g_resendingMessageManagerStorage) ResendingMessageManager();
    return 0;
}

Result ResendingMessageManager::Startup(PacketHandler* packetHandler, bool isRelay)
{
    if (packetHandler == 0)
        return static_cast<Result>(0x10c07);

    m_packetHandler = packetHandler;
    m_isRelay = isRelay;
    return 0;
}

bool ResendingMessageManager::CheckNowResending(unsigned int ackId) const
{
    if (ackId == 0)
        return false;

    for (unsigned int i = 0; i < m_entryNum; ++i)
    {
        if (m_ackIds[i] == ackId)
            return true;
    }
    return false;
}

}}}
