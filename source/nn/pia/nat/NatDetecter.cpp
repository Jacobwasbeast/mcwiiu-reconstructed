#include "nn/pia/nat/NatDetecter.hpp"

#include <new>

#include "nn/pia/common/CallContext.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace nat {

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cReplyTypeFirst = 0x65,
    cProbeReceiveSize = 0x10,
    cAncillarySize = 4,
    cRandomPortAttempts = 100,
    cBindRetryCount = 4,
    cTraceBindFailure = 0x800,
    cDummyProbePort = 0x8236,
    cDummyProbeCount = 5,
    cPortDeltaMax = 1,
    cDetectionPollMsec = 2
};

pead::Random s_natRandom;
}

unsigned short NatDetecter::s_currentPort = 0;
unsigned short NatDetecter::s_randomPortStart = 0;
unsigned short NatDetecter::s_randomPortRange = 0;
unsigned short NatDetecter::s_reservedPort = 0;

NatDetecter::NatDetecter()
    : common::Socket(), m_primaryAddress(), m_senderAddress(), m_probes(), m_detectionJob()
{
    m_sendPayload[0] = 0;
    m_sendPayload[1] = 0;
    m_sendPayload[2] = 0;
    m_sendPayload[3] = 0;
    m_receivePayload[0] = 0;
    m_receivePayload[1] = 0;
    m_receivePayload[2] = 0;
    m_receivePayload[3] = 0;
}

NatDetecter::~NatDetecter()
{
}

unsigned short NatDetecter::GetCurrentPort()
{
    return s_currentPort;
}

void NatDetecter::SetCurrentPort(unsigned short port)
{
    s_currentPort = port;
}

void NatDetecter::SetReservedPort(unsigned int enabled, unsigned short port)
{
    if (enabled != 0)
        return;
    s_reservedPort = port;
}

void NatDetecter::SetRandomPortRange(unsigned short first, unsigned short last)
{
    s_randomPortStart = first;
    s_randomPortRange = last;
}

unsigned int NatDetecter::GetRandomPortNumber()
{
    int attempt = cRandomPortAttempts;
    unsigned int port;
    for (;;)
    {
        const unsigned int end = s_randomPortRange;
        const unsigned int start = s_randomPortStart;
        const unsigned int random = s_natRandom.getU32();
        port = (static_cast<unsigned int>(
                    (static_cast<unsigned long long>(random) * (end - start)) >> 32) +
                start) & 0xffff;
        if (port != s_reservedPort && port != 0)
            break;
        --attempt;
        if (attempt == 0)
            return 0;
    }
    return port;
}

Result NatDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                            const common::InetAddress& address)
{
    if (traverser == 0)
        return cResultInvalidArgument;

    m_retryCount = 0;
    m_primaryAddress = address;
    if (traverser->m_servers.size == 0)
        return cResultInvalidState;

    m_traverser = traverser;
    m_probes.ReleaseAll();
    return 0;
}

Result NatDetecter::CloseSocket()
{
    if (m_openCount < 1)
        return cResultInvalidState;

    const Result result = common::Socket::Close();
    if (result == 0)
        return 0;
    return result;
}

void NatDetecter::Cleanup()
{
    m_probes.ReleaseAll();
    CloseSocket();
    m_traverser = 0;
}

void NatDetecter::Retry()
{
    ++m_retryCount;
    m_probes.ReleaseAll();
}

void NatDetecter::TraceNatCheckReplyArray(unsigned long long)
{
}

unsigned int NatDetecter::BindRandomPort()
{
    for (int attempt = 0; attempt <= cBindRetryCount; ++attempt)
    {
        m_primaryAddress.m_port = static_cast<unsigned short>(GetRandomPortNumber());
        if (common::Socket::Bind(m_primaryAddress, false) == 0)
            return 1;
        m_primaryAddress.Trace(cTraceBindFailure);
    }
    return 0;
}

Result NatDetecter::OpenSocket()
{
    const Result openResult = common::Socket::Open();
    if (openResult != 0)
        return openResult;
    if ((m_primaryAddress.m_port == 0 || common::Socket::Bind(m_primaryAddress, false) != 0) &&
        BindRandomPort() == 0)
    {
        m_primaryAddress.Trace(cTraceBindFailure);
        common::Socket::Close();
        return cResultInvalidState;
    }
    return 0;
}

Result NatDetecter::RetryBind()
{
    Result result = common::Socket::Close();
    if (result != 0)
        return result;
    result = common::Socket::Open();
    if (result != 0)
        return result;
    if (BindRandomPort() == 0)
        return cResultInvalidState;
    return 0;
}

unsigned int NatDetecter::SendNextQueuedProbe()
{
    if (m_probes.m_activeList.GetNodeNum() == 0)
        return 0;

    ProbeNode* probe = reinterpret_cast<ProbeNode*>(
        reinterpret_cast<char*>(m_probes.m_activeList.Sentinel()->next) - m_probes.m_nodeOffset);
    int sentSize = 0;
    m_sendPayload[0] = probe->payload[0];
    m_sendPayload[1] = probe->payload[1];
    m_sendPayload[2] = probe->payload[2];
    m_sendPayload[3] = probe->payload[3];
    const Result result =
        common::Socket::SendTo(m_sendPayload, cProbeReceiveSize, probe->destination, &sentSize);
    if (result != 0)
        return 0;

    probe->destination.~InetAddress();
    m_probes.m_activeList.EraseNode(&probe->node);
    m_probes.m_freeList.InsertAfterNode(m_probes.m_freeList.Sentinel(),
                                        reinterpret_cast<common::ListNode*>(
                                            reinterpret_cast<char*>(probe) + m_probes.m_nodeOffset));
    return 1;
}

unsigned int NatDetecter::SendAllQueuedProbe()
{
    while (m_probes.m_activeList.GetNodeNum() != 0)
    {
        if (SendNextQueuedProbe() == 0)
            return 0;
    }
    return 1;
}

unsigned int NatDetecter::ReceiveNatCheckReply()
{
    int receivedSize = 0;
    unsigned char ancillary[cAncillarySize] = {0};
    const Result result = common::Socket::RecvFrom(
        reinterpret_cast<unsigned char*>(m_receivePayload), cProbeReceiveSize, &m_senderAddress,
        ancillary, &receivedSize);
    if (result == 0 && receivedSize == cProbeReceiveSize)
    {
        const unsigned int index = m_receivePayload[0] - cReplyTypeFirst;
        if (index < cReplyTypeCount)
        {
            NatCheckReply& reply = m_replies[index];
            reply.body.type = m_receivePayload[0];
            reply.body.portWord = m_receivePayload[1];
            reply.body.address = m_receivePayload[2];
            reply.body.payload = m_receivePayload[3];
            ++reply.count;
        }
        return 1;
    }
    return 0;
}

void NatDetecter::PredictPort(common::InetAddress* address)
{
    common::InetAddress predicted(m_replies[0].body.address,
                                  static_cast<unsigned short>(m_replies[0].body.portWord));
    unsigned short delta = static_cast<unsigned short>(m_traverser->m_natProperty.m_portDelta);
    if (delta > cPortDeltaMax)
        delta = cPortDeltaMax;
    predicted.m_port = static_cast<unsigned short>(predicted.m_port + delta);
    if (s_randomPortRange < predicted.m_port)
        predicted.m_port = s_randomPortStart;
    if (address != 0)
        *address = predicted;
}

void NatDetecter::InitializeNatCheckReply()
{
    for (unsigned int index = 0; index < cReplyTypeCount; ++index)
    {
        m_replies[index].count = 0;
        m_replies[index].body.type = 0;
        m_replies[index].body.payload = 0;
        m_replies[index].body.portWord = 0;
        m_replies[index].body.address = 0;
    }
}

void NatDetecter::AddNatCheckProbe(const NatCheckProbe& probe, unsigned short count)
{
    for (; count != 0; --count)
    {
        if (m_probes.m_freeList.GetNodeNum() != 0)
        {
            common::ListNode* freeNode = m_probes.m_freeList.PopBackNode();
            ProbeNode* storage = 0;
            if (freeNode != 0)
                storage = reinterpret_cast<ProbeNode*>(
                    reinterpret_cast<char*>(freeNode) - m_probes.m_nodeOffset);
            ProbeNode* entry = new (storage) ProbeNode();
            m_probes.m_activeList.InsertBeforeNode(m_probes.m_activeList.Sentinel(),
                                                   reinterpret_cast<common::ListNode*>(entry));
            if (entry != 0)
            {
                entry->destination = probe.destination;
                entry->payload[0] = probe.payload[0];
                entry->payload[1] = probe.payload[1];
                entry->payload[2] = probe.payload[2];
                entry->payload[3] = probe.payload[3];
            }
        }
    }
}

common::InetAddress* NatDetecter::GetPrimaryServerPrimaryPortAddress()
{
    if (m_traverser->m_servers.size != 0)
        return &m_traverser->m_servers.data[0];
    return 0;
}

common::InetAddress* NatDetecter::GetPrimaryServerSecondaryPortAddress()
{
    if (m_traverser->m_servers.size > 1)
        return &m_traverser->m_servers.data[1];
    return 0;
}

unsigned short NatDetecter::GetPerceivedPort()
{
    return static_cast<unsigned short>(m_replies[0].body.portWord);
}

NatCheckReply* NatDetecter::GetNatCheckReply(unsigned int index)
{
    return &m_replies[index];
}

void NatDetecter::SendDummyProbe()
{
    common::InetAddress* primary = GetPrimaryServerPrimaryPortAddress();
    common::InetAddress destination(primary->m_address, cDummyProbePort);
    NatCheckProbe probe;
    probe.destination = destination;
    probe.payload[0] = 0;
    probe.payload[1] = 0;
    probe.payload[2] = 0;
    probe.payload[3] = 0;
    AddNatCheckProbe(probe, cDummyProbeCount);
}

unsigned int NatDetecter::CheckRetry()
{
    return 0;
}

void NatDetecter::StartDetectionJob()
{
    m_detectionJob.Ready(true);
}

void NatDetecter::CancelDetectionJob()
{
    m_detectionJob.m_cancelRequested = true;
    m_detectionJob.WaitForCompletion(cDetectionPollMsec);
}

}}}
