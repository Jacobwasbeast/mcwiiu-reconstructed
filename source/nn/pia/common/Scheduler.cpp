#include "nn/pia/common/Scheduler.hpp"

#include "nn/pia/common/BackgroundScheduler.hpp"
#include "nn/pia/common/Job.hpp"

namespace nn { namespace pia { namespace common {

namespace
{
Job* GetJob(ListNode* node, int nodeOffset)
{
    return reinterpret_cast<Job*>(reinterpret_cast<unsigned char*>(node) - nodeOffset);
}

ListNode* GetNode(Job* job, int nodeOffset)
{
    return reinterpret_cast<ListNode*>(reinterpret_cast<unsigned char*>(job) + nodeOffset);
}
}

Scheduler* g_piaJobScheduler = 0;

void Scheduler::EntryJob(Job* job, bool isBackground)
{
    if (isBackground)
    {
        m_backgroundScheduler->EntryJob(job);
        return;
    }

    ListNode* sentinel = m_jobList.Sentinel();
    for (ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (GetJob(node, m_jobNodeOffset)->m_time <= job->m_time)
        {
            m_jobList.InsertBeforeNode(node, GetNode(job, m_jobNodeOffset));
            return;
        }
    }
    m_jobList.InsertBeforeNode(sentinel, GetNode(job, m_jobNodeOffset));
}

void Scheduler::EntryJobNext(Job* job)
{
    m_nextJobList.InsertAfterNode(m_nextJobList.Sentinel(),
                                  GetNode(job, m_nextJobNodeOffset));
}

void Scheduler::ResetJob(Job* job)
{
    if (job->IsForeground())
    {
        ListNode* node = GetNode(job, m_jobNodeOffset);
        if (m_jobList.IsIncludeNode(node))
        {
            m_jobList.EraseNode(node);
            return;
        }
        ListNode* nextNode = GetNode(job, m_nextJobNodeOffset);
        if (m_nextJobList.IsIncludeNode(nextNode))
            m_nextJobList.EraseNode(nextNode);
    }
    else if (job->IsBackground())
    {
        m_backgroundScheduler->ResetJob(job);
    }
}

}}}
