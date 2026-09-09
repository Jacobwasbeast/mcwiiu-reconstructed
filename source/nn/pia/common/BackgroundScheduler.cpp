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

BackgroundScheduler::~BackgroundScheduler()
{
    m_finalize = true;
    m_event.setSignal();
    m_thread.waitDone();
}

void BackgroundScheduler::EntryJob(Job* job)
{
    ListNode* sentinel = m_jobList.Sentinel();
    for (ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (GetJob(node, m_jobNodeOffset)->m_time <= job->m_time)
        {
            m_jobList.InsertBeforeNode(node, GetNode(job, m_jobNodeOffset));
            return;
        }
    }
    m_event.setSignal();
    m_jobList.InsertBeforeNode(sentinel, GetNode(job, m_jobNodeOffset));
}

void BackgroundScheduler::ResetJob(Job* job)
{
    if (m_jobList.IsIncludeNode(GetNode(job, m_jobNodeOffset)))
        m_jobList.EraseNode(GetNode(job, m_jobNodeOffset));
}

}}}
