#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include "nn/pia/common/BackgroundScheduler.hpp"
#include "nn/pia/common/Job.hpp"

namespace nn { namespace pia { namespace common {

Scheduler* g_piaJobScheduler = 0;

void Scheduler::SetMonitoringData()
{
    g_sessionStateMonitoringContent.m_schedulerDispatchCount = m_dispatchCount;
}

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
        if (static_cast<Job*>(m_jobList.FromListNode(node))->m_time <= job->m_time)
        {
            m_jobList.InsertBeforeNode(node, m_jobList.ToListNode(job));
            return;
        }
    }
    m_jobList.InsertBeforeNode(sentinel, m_jobList.ToListNode(job));
}

void Scheduler::EntryJobNext(Job* job)
{
    m_nextJobList.InsertAfterNode(m_nextJobList.Sentinel(), m_nextJobList.ToListNode(job));
}

void Scheduler::ResetJob(Job* job)
{
    if (job->IsForeground())
    {
        if (m_jobList.IsIncludeNode(m_jobList.ToListNode(job)))
        {
            m_jobList.EraseNode(m_jobList.ToListNode(job));
            return;
        }
        if (m_nextJobList.IsIncludeNode(m_nextJobList.ToListNode(job)))
            m_nextJobList.EraseNode(m_nextJobList.ToListNode(job));
    }
    else if (job->IsBackground())
    {
        m_backgroundScheduler->ResetJob(job);
    }
}

}}}
