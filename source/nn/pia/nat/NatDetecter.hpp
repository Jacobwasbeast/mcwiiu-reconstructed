#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_DETECTER_HPP

#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/nat/NatCheckReply.hpp"
#include "nn/pia/nat/NatDetectionJob.hpp"
#include "nn/pia/nat/NatTraverser.hpp"

namespace nn { namespace pia {
namespace common { class CallContext; }
namespace nat {
class NatTraverser;
class NatDetecter;

struct NatCheckProbe
{
    common::InetAddress destination;
    unsigned int payload[4];
};

class NatDetecter : public common::Socket
{
public:
    enum
    {
        cProbeQueueSize = 20,
        cProbePayloadWords = 4,
        cReplyTypeCount = 3
    };

    NatDetecter();
    virtual ~NatDetecter();
    static unsigned short GetCurrentPort();
    static void SetCurrentPort(unsigned short port);
    static void SetReservedPort(unsigned int enabled, unsigned short port);
    static void SetRandomPortRange(unsigned short first, unsigned short last);
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
    void AddNatCheckProbe(const NatCheckProbe& probe, unsigned short count);
    common::InetAddress* GetPrimaryServerPrimaryPortAddress();
    common::InetAddress* GetPrimaryServerSecondaryPortAddress();
    unsigned short GetPerceivedPort();
    NatCheckReply* GetNatCheckReply(unsigned int index);
    void SendDummyProbe();
    virtual void StartSendingNatCheckProbe() = 0;
    virtual bool IsReceivedAllMessage() = 0;
    virtual unsigned int HandleResult() = 0;
    virtual unsigned int CheckRetry();
    virtual unsigned int GetDetectionTimeout() const = 0;
    virtual void StartDetectionJob();
    virtual void CancelDetectionJob();

protected:
    struct ProbeNode
    {
        ProbeNode()
            : destination()
        {
            payload[0] = 0;
            payload[1] = 0;
            payload[2] = 0;
            payload[3] = 0;
        }

        common::ListNode node;
        common::InetAddress destination;
        unsigned int payload[cProbePayloadWords];
    };

    struct ProbeQueue
    {
        ProbeQueue()
            : m_activeList(), m_freeList()
        {
            m_nodeOffset = 0;
            m_nodes = static_cast<ProbeNode*>(static_cast<void*>(m_storage));
            m_nodeCount = cProbeQueueSize;
            for (int index = 0; index < cProbeQueueSize; ++index)
            {
                ProbeNode* probe = static_cast<ProbeNode*>(static_cast<void*>(m_storage[index]));
                probe->node.prev = 0;
                probe->node.next = 0;
                m_freeList.InsertBeforeNode(m_freeList.Sentinel(),
                                            reinterpret_cast<common::ListNode*>(
                                                reinterpret_cast<char*>(probe) + m_nodeOffset));
            }
        }

        void ReleaseAll()
        {
            common::ListNode* sentinel = m_activeList.Sentinel();
            common::ListNode* node = sentinel->next;
            while (node != sentinel)
            {
                common::ListNode* next = node->next;
                node->prev = 0;
                node->next = 0;
                m_freeList.InsertAfterNode(m_freeList.Sentinel(),
                                           reinterpret_cast<common::ListNode*>(
                                               reinterpret_cast<char*>(node) + m_nodeOffset));
                node = next;
            }
            m_activeList.Init();
        }

        common::ListBase m_activeList;
        common::ListBase m_freeList;
        int m_nodeOffset;
        ProbeNode* m_nodes;
        int m_nodeCount;
        unsigned int m_storage[cProbeQueueSize][sizeof(ProbeNode) / sizeof(unsigned int)];
    };

    static unsigned short s_currentPort;
    static unsigned short s_randomPortStart;
    static unsigned short s_randomPortRange;
    static unsigned short s_reservedPort;
    unsigned int m_retryCount;
    common::InetAddress m_primaryAddress;
    common::InetAddress m_senderAddress;
    unsigned int m_sendPayload[cProbePayloadWords];
    unsigned int m_receivePayload[cProbePayloadWords];
    ProbeQueue m_probes;
    NatCheckReply m_replies[cReplyTypeCount];
    unsigned char m_padding324[12];
    NatDetectionJob m_detectionJob;
    NatTraverser* m_traverser;
};

}}}

#endif
