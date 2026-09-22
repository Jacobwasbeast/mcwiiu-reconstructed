#include "nn/pia/nat/NatDetecter.hpp"

#include "platform/wiiu/pead.hpp"

#include "nn/pia/common/CallContext.hpp"

namespace nn { namespace pia { namespace nat {

namespace {
struct NatTraverserServerTableView
{
    unsigned char reserved0[0x12];
    unsigned short portDelta;
    unsigned char reserved14[4];
    unsigned int serverCount;
    common::InetAddress servers[2];
};
}

unsigned short NatDetecter::s_currentPort = 0;
unsigned short NatDetecter::s_randomPortStart = 0;
unsigned short NatDetecter::s_randomPortRange = 0xffff;
unsigned short NatDetecter::s_reservedPort = 0;
static pead::Random s_natRandom;

NatDetecter::NatDetecter()
    : m_traverser(0), m_context(0), m_primaryAddress(), m_secondaryAddress(), m_port(0),
      m_retryCount(0), m_probeCount(0), m_receivedReply(0), m_perceivedPort(0),
      m_detectionJob(), m_cancelRequested(false), m_activeProbes(), m_freeProbes(),
      m_probeNodes(), m_replies()
{
    m_activeProbes.Init();
    m_freeProbes.Init();
    for (unsigned int i = 0; i < 20; ++i)
        m_freeProbes.InsertBeforeNode(m_freeProbes.Sentinel(), &m_probeNodes[i].node);
    InitializeNatCheckReply();
}

NatDetecter::~NatDetecter()
{
    if (m_detectionJob.IsRunning())
        m_detectionJob.CancelCleanup();
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
    if (enabled == 0)
        s_reservedPort = port;
}

void NatDetecter::SetRandomPortRange(unsigned short first, unsigned short count)
{
    s_randomPortStart = first;
    s_randomPortRange = count;
}

unsigned int NatDetecter::GetRandomPortNumber()
{
    for (int attempt = 100; attempt > 0; --attempt)
    {
        const unsigned int first = s_randomPortStart;
        const unsigned int last = s_randomPortRange;
        const unsigned int span = last - first;
        const unsigned int random = s_natRandom.getU32();
        const unsigned int scaled = static_cast<unsigned int>(
            (static_cast<unsigned long long>(random) * span) >> 32);
        const unsigned int value = first + scaled;
        if (value != s_reservedPort && value != 0)
            return value & 0xffffu;
    }
    return 0;
}

Result NatDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                            const common::InetAddress& address)
{
    if (context == 0)
        return 0x10c07;
    m_traverser = traverser;
    m_context = context;
    m_primaryAddress = address;
    m_detectionJob.Startup(context, this, traverser);
    if (context->m_state != common::CallContext::cStateInProgress)
        return 0x10c08;
    m_retryCount = 0;
    while (common::ListNode* node = m_activeProbes.PopFrontNode())
        m_freeProbes.InsertAfterNode(m_freeProbes.Sentinel(), node);
    m_probeCount = 0;
    InitializeNatCheckReply();
    return 0;
}

Result NatDetecter::CloseSocket()
{
    return common::Socket::Close();
}

void NatDetecter::Cleanup()
{
    CloseSocket();
    m_traverser = 0;
    m_context = 0;
    m_detectionJob.CancelCleanup();
    m_cancelRequested = false;
    m_probeCount = 0;
    m_receivedReply = 0;
    while (common::ListNode* node = m_activeProbes.PopFrontNode())
        m_freeProbes.InsertAfterNode(m_freeProbes.Sentinel(), node);
}

void NatDetecter::Retry()
{
    ++m_retryCount;
    m_probeCount = 0;
    m_receivedReply = 0;
    while (common::ListNode* node = m_activeProbes.PopFrontNode())
        m_freeProbes.InsertAfterNode(m_freeProbes.Sentinel(), node);
}

void NatDetecter::TraceNatCheckReplyArray(unsigned long long value)
{
    (void)value;
}

unsigned int NatDetecter::BindRandomPort()
{
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        m_port = static_cast<unsigned short>(GetRandomPortNumber());
        m_primaryAddress.m_port = m_port;
        if (common::Socket::Bind(m_primaryAddress, false) == 0)
        {
            return 1;
        }
    }
    return 0;
}

Result NatDetecter::OpenSocket()
{
    common::Socket::Open();
    Result bindResult = 0;
    if (m_port == 0)
        bindResult = BindRandomPort() == 0 ? 0x10c08 : 0;
    else
        bindResult = common::Socket::Bind(m_primaryAddress, false);
    if (bindResult != 0)
    {
        common::Socket::Close();
        return bindResult;
    }
    return 0;
}

Result NatDetecter::RetryBind()
{
    Result result = common::Socket::Close();
    if (result != 0)
        return result;
    common::Socket::Open();
    return BindRandomPort() == 0 ? 0x10c08 : 0;
}

unsigned int NatDetecter::SendNextQueuedProbe()
{
    unsigned char packet[16] = {};
    int sentSize = 0;
    common::InetAddress destination = m_primaryAddress;
    const void* payload = packet;
    QueuedProbe* queued = 0;
    common::ListNode* node = m_activeProbes.Sentinel()->next;
    if (node != m_activeProbes.Sentinel())
    {
        queued = reinterpret_cast<QueuedProbe*>(node);
        destination = queued->destination;
        payload = queued->payload;
    }
    const int result = common::Socket::SendTo(payload, 16, destination, &sentSize);
    if (result != 0)
        return 0;
    if (queued != 0)
    {
        m_activeProbes.EraseNode(&queued->node);
        m_freeProbes.InsertAfterNode(m_freeProbes.Sentinel(), &queued->node);
    }
    if (queued != 0 && m_probeCount != 0)
        --m_probeCount;
    return 1;
}

unsigned int NatDetecter::SendAllQueuedProbe()
{
    while (m_probeCount != 0)
    {
        if (SendNextQueuedProbe() == 0)
            return 0;
    }
    return 1;
}

unsigned int NatDetecter::ReceiveNatCheckReply()
{
    unsigned char packet[16] = {};
    unsigned char ancillary[16] = {};
    int ancillarySize = 0;
    int received = 0;
    if (common::Socket::RecvFrom(packet, 16, &m_secondaryAddress, ancillary, &ancillarySize) != 0 || ancillarySize != 16)
        return 0;
    int type = 0;
    std::memcpy(&type, packet, sizeof(type));
    if (type < 0x65 || type > 0x67)
        return 0;
    NatCheckReply& reply = m_replies[type - 0x65];
    std::memcpy(&reply.type, packet, sizeof(reply.type));
    std::memcpy(&reply.sourceAddress, packet + 4, sizeof(reply.sourceAddress));
    std::memcpy(&reply.sourcePort, packet + 8, sizeof(reply.sourcePort));
    std::memcpy(&reply.payload, packet + 12, sizeof(reply.payload));
    ++reply.count;
    ++m_receivedReply;
    return 1;
}

void NatDetecter::PredictPort(common::InetAddress* address)
{
    common::InetAddress predicted = m_primaryAddress;
    unsigned short offset = 0;
    if (m_traverser != 0)
    {
        const NatTraverserServerTableView* state =
            reinterpret_cast<const NatTraverserServerTableView*>(m_traverser);
        offset = state->portDelta;
        if (offset > 1)
            offset = 1;
    }
    predicted.m_port = static_cast<unsigned short>(predicted.m_port + offset);
    if (predicted.m_port > s_randomPortRange)
        predicted.m_port = s_randomPortStart;
    if (address != 0)
        *address = predicted;
}

void NatDetecter::InitializeNatCheckReply()
{
    for (unsigned int i = 0; i < 3; ++i)
    {
        m_replies[i].count = 0;
        m_replies[i].type = 0;
        m_replies[i].sourceAddress = 0;
        m_replies[i].sourcePort = 0;
        m_replies[i].payload = 0;
    }
    m_receivedReply = 0;
}

void NatDetecter::AddNatCheckProbe(const NatCheckProbe& probe, unsigned short type)
{
    for (unsigned int n = 0; n < type; ++n)
    {
        common::ListNode* node = m_freeProbes.PopBackNode();
        if (node == 0)
            break;
        QueuedProbe* target = reinterpret_cast<QueuedProbe*>(node);
        target->destination = probe.destination;
        std::memcpy(target->payload, probe.payload, sizeof(target->payload));
        target->fields[0] = type;
        target->fields[1] = 0;
        m_activeProbes.InsertBeforeNode(m_activeProbes.Sentinel(), &target->node);
        ++m_probeCount;
    }
}

common::InetAddress* NatDetecter::GetPrimaryServerPrimaryPortAddress()
{
    if (m_traverser == 0)
        return 0;
    NatTraverserServerTableView* state = reinterpret_cast<NatTraverserServerTableView*>(m_traverser);
    return state->serverCount == 0 ? 0 : &state->servers[0];
}

common::InetAddress* NatDetecter::GetPrimaryServerSecondaryPortAddress()
{
    if (m_traverser == 0)
        return 0;
    NatTraverserServerTableView* state = reinterpret_cast<NatTraverserServerTableView*>(m_traverser);
    return state->serverCount > 1 ? &state->servers[1] : 0;
}

unsigned short NatDetecter::GetPerceivedPort()
{
    return m_perceivedPort;
}

NatDetecter::NatCheckReply* NatDetecter::GetNatCheckReply(unsigned int index)
{
    return &m_replies[index];
}

void NatDetecter::StartDetectionJob()
{
    m_detectionJob.Ready(true);
    m_cancelRequested = false;
}

void NatDetecter::CancelDetectionJob()
{
    m_cancelRequested = true;
    m_detectionJob.m_cancelRequested = true;
    m_detectionJob.WaitForCompletion(2);
}

void NatDetecter::SendDummyProbe()
{
    common::InetAddress* primary = GetPrimaryServerPrimaryPortAddress();
    NatCheckProbe probe = {};
    probe.destination = *primary;
    probe.destination.m_port = 0x8236;
    AddNatCheckProbe(probe, 5);
}

unsigned int NatDetecter::CheckRetry()
{
    return 0;
}

}}}
