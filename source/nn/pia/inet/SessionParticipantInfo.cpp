#include "nn/pia/inet/SessionParticipantInfo.hpp"

namespace nn { namespace pia { namespace inet {

namespace
{

enum
{
    cParticipationEventDisconnect = 7,
    cParticipationEventEndParticipation = 8
};

}

SessionParticipantInfo::SessionParticipantInfo()
    : m_sessionId(0)
{
    m_participantList.Clear();
}

SessionParticipantInfo::~SessionParticipantInfo()
{
    Cleanup();
}

void SessionParticipantInfo::Cleanup()
{
    m_sessionId = 0;
    m_participantList.Clear();
}

void SessionParticipantInfo::RemoveObsoleteEntry()
{
    ParticipantList& list = m_participantList;
    common::ListNode* end = list.Sentinel();
    common::ListNode* node = end->next;
    common::ListNode* next = node->next;
    while (node != end)
    {
        ParticipantList::Node* participant = static_cast<ParticipantList::Node*>(node);
        if (participant->value.m_participationCount < 0 &&
            (participant->value.m_participationEvent == cParticipationEventDisconnect ||
             participant->value.m_participationEvent == cParticipationEventEndParticipation))
            list.Free(participant);
        node = next;
        next = node->next;
    }
    if (list.GetNodeNum() == 0)
        m_sessionId = 0;
}

}}}
