#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTER_HPP

#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/nat/NatDetectionJob.hpp"
#include "nn/pia/nat/NatTraverser.hpp"

#include <cstring>

namespace nn { namespace pia {
namespace common { class CallContext; }
namespace nat {
class NatTraverser;
class NatDetecter;
struct NatCheckProbe
{
    unsigned int reserved[2];
    common::InetAddress destination;
    unsigned int payload[4];
};

class NatDetecter : public common::Socket
{
public:
    NatDetecter();
    virtual ~NatDetecter();
    static unsigned short GetCurrentPort();
    static void SetCurrentPort(unsigned short port);
    static void SetReservedPort(unsigned int enabled, unsigned short port);
    static void SetRandomPortRange(unsigned short first, unsigned short count);
    static unsigned int GetRandomPortNumber();
    virtual Result Startup(NatTraverser* traverser, common::CallContext* context,
                   const common::InetAddress& address);
    Result CloseSocket();
    virtual void Cleanup();
    virtual void Retry();
    void TraceNatCheckReplyArray(unsigned long long value);
    unsigned int BindRandomPort();
    Result OpenSocket();
    Result RetryBind();
    unsigned int SendNextQueuedProbe();
    unsigned int SendAllQueuedProbe();
    unsigned int ReceiveNatCheckReply();
    void PredictPort(common::InetAddress* address);
    void InitializeNatCheckReply();
    void AddNatCheckProbe(const NatCheckProbe& probe, unsigned short type);
    common::InetAddress* GetPrimaryServerPrimaryPortAddress();
    common::InetAddress* GetPrimaryServerSecondaryPortAddress();
    unsigned short GetPerceivedPort();
    struct NatCheckReply
    {
        int count;
        int type;
        int sourceAddress;
        int sourcePort;
        int payload;
    };
    NatCheckReply* GetNatCheckReply(unsigned int index);
    void SendDummyProbe();
    virtual void StartSendingNatCheckProbe() = 0;
    virtual bool IsReceivedAllMessage() = 0;
    virtual unsigned int HandleResult() = 0;
    virtual unsigned int CheckRetry();
    virtual unsigned int GetDetectionTimeout() const = 0;
    virtual void StartDetectionJob();
    virtual void CancelDetectionJob();

private:
    static unsigned short s_currentPort;
    static unsigned short s_randomPortStart;
    static unsigned short s_randomPortRange;
    static unsigned short s_reservedPort;
    common::InetAddress m_primaryAddress;
    common::InetAddress m_secondaryAddress;
    unsigned short m_port;
    unsigned int m_retryCount;
    unsigned int m_probeCount;
    unsigned int m_receivedReply;
    unsigned short m_perceivedPort;
    unsigned char m_preListPadding[0x14];
    common::ListBase m_activeProbes;
    common::ListBase m_freeProbes;
    struct QueuedProbe
    {
        common::ListNode node;
        common::InetAddress destination;
        unsigned char payload[16];
        int fields[2];
    };
    QueuedProbe m_probeNodes[20];
    unsigned char m_replyPadding[8];
    NatCheckReply m_replies[3];
    unsigned char m_jobPadding[12];
    NatDetectionJob m_detectionJob;
    NatTraverser* m_traverser;
    common::CallContext* m_context;
    bool m_cancelRequested;
};

}}}

#endif
