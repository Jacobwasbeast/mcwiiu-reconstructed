#include "nn/pia/common/BackgroundScheduler.hpp"

#include "nn/pia/common/Job.hpp"

#include <stddef.h>

namespace nn { namespace pia { namespace common {

namespace
{
const int kThreadQueueCapacity = 0x7fffffff;
const int kThreadStackSize = 0x5000;
const int kThreadAffinity = 0x20;
}

BackgroundScheduler::BackgroundScheduler(int threadPriority,
                                         CriticalSection* criticalSection)
    : m_delegate(this, &BackgroundScheduler::Dispatch),
      m_thread("Tc@Pia BackgroundScheduler", &m_delegate,
               common::HeapManager::GetHeap(), threadPriority,
               pead::MessageQueue::cNoBlock, kThreadQueueCapacity,
               kThreadStackSize, kThreadAffinity),
      m_criticalSection(criticalSection),
      m_event(false),
      m_finalize(false),
      m_jobList()
{
    m_jobList.Init();
    m_thread.start();
}

namespace
{
Job* GetJob(ListNode* node)
{
    return reinterpret_cast<Job*>(reinterpret_cast<unsigned char*>(node) -
                                  offsetof(Job, m_listNode));
}

ListNode* GetNode(Job* job)
{
    return &job->m_listNode;
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
        if (GetJob(node)->m_time <= job->m_time)
        {
            m_jobList.InsertBeforeNode(node, GetNode(job));
            return;
        }
    }
    m_event.setSignal();
    m_jobList.InsertBeforeNode(sentinel, GetNode(job));
}

void BackgroundScheduler::ResetJob(Job* job)
{
    if (m_jobList.IsIncludeNode(GetNode(job)))
        m_jobList.EraseNode(GetNode(job));
}

void BackgroundScheduler::Dispatch(pead::Thread* thread, int message)
{
    if (thread == 0)
        return;
    for (;;)
    {
        if (m_finalize)
            return;

        ListNode* node = m_jobList.PopFrontNode();
        if (node == 0)
        {
            m_event.wait();
            continue;
        }

        Job* job = GetJob(node);
        job->Execute(message != 0);
    }
}

}}}
