#include "nn/pia/lan/LanProtocol.hpp"

#include <new>
#include <stdint.h>

#include <coreinit/memory.h>

#include "nn/pia/common/AesImpl.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/lan/LanInputStream.hpp"
#include "nn/pia/lan/LanMatchmakeUpdateJob.hpp"
#include "nn/pia/lan/LanOutputStream.hpp"
#include "nn/pia/lan/LanSelfKeepAliveMessage.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
const unsigned int cSelfKeepAliveIntervalMsec = 2000;
const unsigned int cRequestTimeoutMsec = 2000;
const unsigned int cSelfKeepAliveMessageType = 7;
const unsigned int cGetHostMessageType = 3;
const unsigned int cHostMessageType = 4;
const unsigned int cGetSessionMessageType = 5;
const unsigned int cSessionMessageType = 6;
const unsigned int cKeySize = 16;
const unsigned int cMatchmakeUpdateJobSize = 0xe90;

template <typename T> T* newObject()
{
    void* storage = pead::allocateFromSeadHeap(sizeof(T), common::HeapManager::GetHeap());
    return storage == 0 ? 0 : new (storage) T();
}

template <typename T> void deleteObject(T* object)
{
    object->~T();
    pead::freeToSeadHeap(object);
}
}

void LanProtocol::ReceivedFragmentData::Clear()
{
    OSBlockSet(m_data, 0, sizeof(m_data));
    m_fragmentCount = 0;
    m_receivedMask = 0;
    m_sequence = 0;
    m_id = 0;
    m_source.Clear();
}

void LanProtocol::ReceivedFragmentData::Receive(const void* data, unsigned int size,
                                                unsigned int id, unsigned short sequence,
                                                unsigned char index, unsigned char count,
                                                const common::StationAddress& source)
{
    if (m_id != id || m_sequence != sequence || m_fragmentCount != count ||
        !(source == m_source))
    {
        Clear();
        m_id = id;
        m_sequence = sequence;
        m_fragmentCount = count;
        m_source = source;
    }
    OSBlockMove(m_data + m_fragmentSize * index, data, size, FALSE);
    m_receivedMask |= static_cast<unsigned char>(1 << index);
}

bool LanProtocol::ReceivedFragmentData::IsReceivedAll() const
{
    return m_receivedMask == (1 << m_fragmentCount) - 1;
}

LanProtocol::LanProtocol()
    : transport::Protocol(),
      m_started(0), m_address(0), m_broadcastAddress(0), m_port(0), m_unknown22(0),
      m_unknown24(0), m_socket(0), m_broadcastSocket(0), m_broadcastSocketBound(0),
      m_outputStream(0), m_inputStream(0), m_matchmakeUpdateJob(0), m_unknown618(0),
      m_receivedSessionInfo(), m_sessionMessageId(0), m_getHostContext(), m_getHostDeadline(),
      m_getSessionContext(), m_getSessionDeadline(), m_updateSessionContext(),
      m_selfKeepAliveTime(), m_cryptoSetting(), m_random()
{
    LanFacade::GetInstance()->RegisterLanProtocol(this);
    static_cast<LanInputStream*>(transport::g_piaThreadStreamManager->m_packetInput)
        ->RegisterLanProtocol(this);
    static_cast<LanOutputStream*>(transport::g_piaThreadStreamManager->m_packetOutput)
        ->RegisterLanProtocol(this);
}

LanProtocol::~LanProtocol()
{
    if (transport::g_piaThreadStreamManager != 0)
    {
        LanOutputStream* output = static_cast<LanOutputStream*>(
            transport::g_piaThreadStreamManager->m_packetOutput);
        if (output != 0)
            output->UnregisterLanProtocol();
        LanInputStream* input = static_cast<LanInputStream*>(
            transport::g_piaThreadStreamManager->m_packetInput);
        if (input != 0)
            input->UnregisterLanProtocol();
    }
    LanFacade::GetInstance()->Cleanup();
    LanFacade::GetInstance()->UnregisterLanProtocol();
}

Result LanProtocol::Initialize()
{
    m_started = 0;
    m_socket = newObject<common::Socket>();
    m_broadcastSocket = newObject<common::Socket>();
    void* storage = pead::allocateFromSeadHeap(cMatchmakeUpdateJobSize,
                                               common::HeapManager::GetHeap());
    if (storage != 0)
        m_matchmakeUpdateJob = new (storage) LanMatchmakeUpdateJob(this);
    else
        m_matchmakeUpdateJob = 0;
    m_randomSeedSet = 0;
    m_sessionKeyRegistered = 0;
    m_cryptoSettingRegistered = 0;
    m_sessionKeyRequested = 0;
    m_cryptoSetting.m_mode = 1;
    m_unknown24 = 0;
    m_broadcastSocketBound = 0;
    return 0;
}

void LanProtocol::Finalize()
{
    if (m_socket != 0)
    {
        deleteObject(m_socket);
        m_socket = 0;
    }
    if (m_broadcastSocket != 0)
    {
        deleteObject(m_broadcastSocket);
        m_broadcastSocket = 0;
    }
    if (m_matchmakeUpdateJob != 0)
    {
        m_matchmakeUpdateJob->~LanMatchmakeUpdateJob();
        pead::freeToSeadHeap(m_matchmakeUpdateJob);
        m_matchmakeUpdateJob = 0;
    }
}

void LanProtocol::Trace(unsigned long long) const
{
}

unsigned short LanProtocol::GetProtocolType() const
{
    return cProtocolType;
}

unsigned int LanProtocol::IsProtocolFilteringEnabled() const
{
    return 0;
}

void LanProtocol::ResetSelfKeepAliveTime()
{
    const unsigned long long interval =
        common::TimeSpan::GetTicksPerMSec().m_tick * cSelfKeepAliveIntervalMsec;
    common::Time now;
    now.SetNow();
    m_selfKeepAliveTime.m_tick = now.m_tick + interval;
}

Result LanProtocol::Startup()
{
    m_getHostContext.Reset();
    m_getSessionContext.Reset();
    m_updateSessionContext.Reset();
    ResetSelfKeepAliveTime();
    m_unknown618 = 0;
    m_receivedSessionInfo.Clear();
    m_sessionMessageId = 0;
    return 0;
}

void LanProtocol::Cleanup()
{
    if (m_getHostContext.m_state == common::CallContext::cStateInProgress)
        m_getHostContext.SignalCancel();
    if (m_getSessionContext.m_state == common::CallContext::cStateInProgress)
        m_getSessionContext.SignalCancel();
    if (m_updateSessionContext.m_state == common::CallContext::cStateInProgress)
        m_updateSessionContext.SignalCancel();
}

bool LanProtocol::IsCryptoEnabled() const
{
    return m_cryptoSetting.m_mode != 0;
}

void LanProtocol::SendMessage(LanPayload* payload, const common::StationAddress& address,
                              bool isPriority, bool field602, unsigned long long)
{
    if (IsCryptoEnabled() && m_sessionKeyRegistered == 0 &&
        payload->m_type != cSelfKeepAliveMessageType)
        return;

    transport::PacketHandler* handler = reinterpret_cast<transport::PacketHandler*>(
        static_cast<uintptr_t>(mStationPacketHandler));
    transport::ProtocolMessageWriter* writer = handler->AssignByStationAddress(
        transport::ProtocolId(cProtocolId), address, payload->m_size, isPriority, field602);
    if (writer != 0)
    {
        writer->SetPayload(payload->m_data);
        handler->Commit();
    }
}

void LanProtocol::ParseSelfKeepAlive(const LanPayload* payload, const common::StationAddress&)
{
    LanSelfKeepAliveMessage message;
    message.Deserialize(payload->m_data);
}

void LanProtocol::ParseMessage(const LanPayload* payload, const common::StationAddress& source)
{
    switch (payload->m_type)
    {
    case cGetHostMessageType:
        ParseGetHostMessage(payload, source);
        break;
    case cHostMessageType:
        ParseHostMessage(payload, source);
        break;
    case cGetSessionMessageType:
        ParseGetSessionMessage(payload, source);
        break;
    case cSessionMessageType:
        ParseSessionMessage(payload, source);
        break;
    case cSelfKeepAliveMessageType:
        ParseSelfKeepAlive(payload, source);
        break;
    }
}

Result LanProtocol::Bind(const LanFacade::LanNetworkSetting& setting)
{
    if (setting.m_field00 == 0)
        return 0x10c07;

    m_address = setting.m_field00;
    m_broadcastAddress = (setting.m_field00 & setting.m_field04) | ~setting.m_field04;
    m_unknown24 = setting.m_field08;
    return 0;
}

void LanProtocol::Unbind()
{
    m_unknown24 = 0;
    m_unknown22 = 0;
    m_address = 0;
    m_port = 0;
    m_broadcastAddress = 0;
}

void LanProtocol::RegisterCryptoSetting(const common::CryptoSetting& setting)
{
    m_cryptoSetting.m_mode = setting.m_mode;
    OSBlockMove(m_cryptoSetting.m_key, setting.m_key, cKeySize, FALSE);
    m_cryptoSettingRegistered = 1;
}

void LanProtocol::UnregisterCryptoSetting()
{
    m_cryptoSetting.m_mode = common::Crypto::cModeAesGcm;
    OSBlockSet(m_cryptoSetting.m_key, 0, cKeySize);
    m_cryptoSettingRegistered = 0;
}

unsigned int LanProtocol::RequestBrowseMessage(unsigned char type)
{
    return m_matchmakeUpdateJob->RequestBrowseMessage(type);
}

bool LanProtocol::IsBrowseMessageCompleted() const
{
    return m_matchmakeUpdateJob->IsBrowseMessageCompleted();
}

bool LanProtocol::RequestGetHostMessage()
{
    if (m_getHostContext.m_state == common::CallContext::cStateInProgress)
        return false;

    m_getHostContext.Reset();
    m_getHostContext.InitiateCall();
    if (!session::g_piaMesh->IsProcessingLeaveMesh())
        SendGetHostMessage();

    const unsigned long long interval =
        common::TimeSpan::GetTicksPerMSec().m_tick * cRequestTimeoutMsec;
    common::Time now;
    now.SetNow();
    m_getHostDeadline.m_tick = now.m_tick + interval;
    return true;
}

bool LanProtocol::IsGetHostMessageCompleted() const
{
    return m_getHostContext.m_state == common::CallContext::cStateSuccess ||
           m_getHostContext.m_state == common::CallContext::cStateFailure ||
           m_getHostContext.m_state == common::CallContext::cStateCancelled;
}

bool LanProtocol::RequestUpdateSessionMessage()
{
    if (m_updateSessionContext.m_state == common::CallContext::cStateInProgress)
        return false;

    m_updateSessionContext.Reset();
    m_updateSessionContext.InitiateCall();
    return true;
}

bool LanProtocol::IsUpdateSessionMessageCompleted() const
{
    return m_updateSessionContext.m_state == common::CallContext::cStateSuccess ||
           m_updateSessionContext.m_state == common::CallContext::cStateFailure ||
           m_updateSessionContext.m_state == common::CallContext::cStateCancelled;
}

bool LanProtocol::RequestGetSessionMessage()
{
    if (m_getSessionContext.m_state == common::CallContext::cStateInProgress)
        return false;

    m_getSessionContext.Reset();
    m_getSessionContext.InitiateCall();
    SendGetSessionMessage();

    const unsigned long long interval =
        common::TimeSpan::GetTicksPerMSec().m_tick * cRequestTimeoutMsec;
    common::Time now;
    now.SetNow();
    m_getSessionDeadline.m_tick = now.m_tick + interval;
    return true;
}

bool LanProtocol::IsGetSessionMessageCompleted() const
{
    return m_getSessionContext.m_state == common::CallContext::cStateSuccess ||
           m_getSessionContext.m_state == common::CallContext::cStateFailure ||
           m_getSessionContext.m_state == common::CallContext::cStateCancelled;
}

Result LanProtocol::GetSessionMessageResult() const
{
    return m_getSessionContext.m_result;
}

Result LanProtocol::CalculateCryptionKeyForBrowse(const unsigned char* input,
                                                  unsigned int inputSize, void* output,
                                                  unsigned int outputSize)
{
    if (input == 0 || inputSize != cKeySize || output == 0 || outputSize != cKeySize)
        return 0x10c07;
    if (m_cryptoSetting.m_mode == 0)
        return 0x10c08;

    common::AesImpl<common::XCUiL_2_16> aes;
    aes.Initialize(m_cryptoSetting.m_key, cKeySize, true);
    aes.EncryptBlock(output, cKeySize, input, cKeySize);
    return 0;
}

unsigned char LanProtocol::IsCryptoSettingRegistered() const
{
    return m_cryptoSettingRegistered;
}

void LanProtocol::SetRandomSeed(unsigned long long first, unsigned long long second)
{
    m_random.UpdateSetting(first, second);
    m_randomSeedSet = 1;
}

transport::StationConnectionInfo& LanProtocol::GetLocalStationConnectionInfo()
{
    return *transport::g_stationConnectionInfoTable;
}

void LanProtocol::RequestRegisterSessionKey()
{
    m_sessionKeyRequested = 1;
}

}}}
