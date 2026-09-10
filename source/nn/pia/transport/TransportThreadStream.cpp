#include "nn/pia/transport/TransportThreadStream.hpp"

#include "nn/pia/transport/LatencyEmulator.hpp"

namespace nn { namespace pia { namespace transport {

TransportThreadStream::TransportThreadStream()
    : PacketStream(),
      m_threadDelegate(this, &TransportThreadStream::ThreadFunc),
      m_thread(0), m_event(false), m_threadState(cThreadStateQuit),
      m_sleepMilliSecond(5), m_lastResult(0), m_latencyEmulator(0),
      m_packetLossRatio(0), m_isPacketLossEnabled(false)
{
}

TransportThreadStream::~TransportThreadStream()
{
}

void TransportThreadStream::ChangeState(ThreadState threadState)
{
    if (m_threadState == threadState)
        return;

    pead::TickSpan interval;
    interval.setNanoSeconds(100000);

    do
    {
        m_threadState = static_cast<ThreadState>(threadState | cThreadStateRequest);
        m_event.setSignal();
        while (m_threadState != threadState)
        {
            pead::Thread::sleep(interval);
            if (m_threadState == cThreadStateError)
                break;
        }
        if (m_threadState == threadState)
            return;
    }
    while (threadState != cThreadStateRun);
}

Result TransportThreadStream::Startup()
{
    if (m_threadState != cThreadStateStop)
        return static_cast<Result>(0x10c08);

    if (m_latencyEmulator != 0)
        m_latencyEmulator->Clear();

    PacketStream::Startup();
    m_lastResult = 0;
    OnStartup();
    ChangeState(cThreadStateRun);
    return 0;
}

void TransportThreadStream::Cleanup()
{
    if (m_threadState == cThreadStateQuit)
        return;

    ChangeState(cThreadStateStop);
    PacketStream::Cleanup();
    if (m_latencyEmulator != 0)
        m_latencyEmulator->Clear();
}

Result TransportThreadStream::GetLastResult() const
{
    if (m_threadState == cThreadStateStop || m_threadState == cThreadStateError)
        return m_lastResult;
    return 0;
}

void TransportThreadStream::ThreadFunc(pead::Thread* thread, int)
{
    for (;;)
    {
        const ThreadState threadState = m_threadState;
        if (threadState == static_cast<ThreadState>(cThreadStateStop | cThreadStateRequest))
        {
            m_threadState = cThreadStateStop;
            continue;
        }
        if (threadState == static_cast<ThreadState>(cThreadStateRun | cThreadStateRequest))
        {
            m_threadState = cThreadStateRun;
            continue;
        }
        if (threadState == static_cast<ThreadState>(cThreadStateQuit | cThreadStateRequest))
        {
            thread->quit(false);
            m_threadState = cThreadStateQuit;
            return;
        }
        if (threadState > cThreadStateRequest)
            continue;

        if (threadState == cThreadStateRun)
        {
            Result result = ProcessOne();
            if (m_latencyEmulator != 0)
            {
                const Result dispatchResult = m_latencyEmulator->Dispatch();
                if (dispatchResult != 0)
                    result = dispatchResult;
            }
            if (result == 0 || result == static_cast<Result>(0x2c03))
                continue;

            if (result == static_cast<Result>(0x4c0d) ||
                result == static_cast<Result>(0x2c09))
            {
                pead::TickSpan interval;
                interval.setNanoSeconds(static_cast<long long>(m_sleepMilliSecond) * 1000000);
                m_event.wait(interval);
            }
            else
            {
                m_lastResult = result;
                m_threadState = cThreadStateError;
            }
            continue;
        }

        if (threadState == cThreadStateStop || threadState == cThreadStateError)
            m_event.wait();
    }
}

bool TransportThreadStream::isDropPacket()
{
    if (!m_isPacketLossEnabled)
        return false;
    if (m_packetLossRatio == 0)
        return false;

    const unsigned long long scaled =
        static_cast<unsigned long long>(m_random.getU32()) * 100;
    return static_cast<int>(scaled >> 32) < m_packetLossRatio;
}

void TransportThreadStream::OnStartup()
{
}

}}}
