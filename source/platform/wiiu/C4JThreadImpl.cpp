#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/System.hpp"
#include "game/core/CriticalSection.hpp"
#include <coreinit/memdefaultheap.h>
#include <coreinit/fastmutex.h>
#include <coreinit/atomic64.h>
#include <coreinit/atomic.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <coreinit/systeminfo.h>
#include <stdio.h>
#include <vector>

static std::vector<C4JThreadImpl*> s_threads;
static C4JThreadImpl s_mainThread("Main thread");
static OSFastMutex s_createSection;
static OSFastMutex s_threadSection;
static bool s_createSectionInitialized = false;
static __thread uint32_t s_specialThreadIDForFS_MilesStreamThread;
static const unsigned char s_processorAffinity[3] = { 1, 2, 4 };
static const unsigned char s_priorities[] = { 0x10, 0x0F, 0x0E, 0x00 };

void SetThreadName(unsigned int, const char*)
{
}

long long InterlockedCompareExchangeRelease64(volatile long long* destination,
                                              long long exchange,
                                              long long comparand)
{
    uint64_t previous = 0;
    OSCompareAndSwapAtomicEx64(reinterpret_cast<volatile uint64_t*>(destination),
                               static_cast<uint64_t>(comparand),
                               static_cast<uint64_t>(exchange), &previous);
    return static_cast<long long>(previous);
}

long InterlockedCompareExchangeRelease(volatile long* destination,
                                        long exchange, long comparand)
{
    uint32_t previous = 0;
    OSCompareAndSwapAtomicEx(reinterpret_cast<volatile uint32_t*>(destination),
                              static_cast<uint32_t>(comparand),
                              static_cast<uint32_t>(exchange), &previous);
    return static_cast<long>(previous);
}

typedef std::vector<C4JThreadImpl*> C4JThreadVector;
typedef C4JThreadVector::iterator C4JThreadVectorIterator;

void Sleep(unsigned int milliseconds)
{
    const uint64_t ticks = static_cast<uint64_t>(milliseconds) *
                           static_cast<uint64_t>(OSGetSystemInfo()->busClockSpeed >> 2) /
                           1000ull;
    OSSleepTicks(ticks);
}

uint32_t C4JThreadImpl::getCurrentThread()
{
    OSThread* currentThread = OSGetCurrentThread();
    EnterCriticalSection(&s_threadSection);
    for (size_t i = 0; i < s_threads.size(); ++i)
    {
        if (s_threads[i]->m_thread == currentThread)
        {
            LeaveCriticalSection(&s_threadSection);
            return (uint32_t)s_threads[i];
        }
    }
    LeaveCriticalSection(&s_threadSection);
    return 0;
}

bool C4JThreadImpl::isMainThread()
{
    return getCurrentThread() == (uint32_t)&s_mainThread;
}

const char* C4JThreadImpl::getCurrentThreadName()
{
    return ((C4JThreadImpl*)getCurrentThread())->getName();
}

void C4JThreadImpl::threadDeallocator(void*, void* stack)
{
    MEMFreeToDefaultHeap(stack);
}

uint32_t C4JThreadImpl::OSCreateThread4J(OSThread* thread, int (*entry)(int, void*), int argumentCount, void* argument, void* stack, unsigned int stackSize, int priority, unsigned short attributes)
{
    if (!s_createSectionInitialized)
    {
        InitializeCriticalSection(&s_createSection);
        s_createSectionInitialized = true;
    }
    EnterCriticalSection(&s_createSection);
    uint32_t result = OSCreateThread(thread, reinterpret_cast<OSThreadEntryPointFn>(entry), argumentCount, static_cast<char*>(argument), stack, stackSize, priority, static_cast<OSThreadAttributes>(attributes));
    OSSetThreadDeallocator(thread, reinterpret_cast<OSThreadDeallocatorFn>(threadDeallocator));
    LeaveCriticalSection(&s_createSection);
    return result;
}

int C4JThreadImpl::entryPoint(int, void* argument)
{
    C4JThreadImpl* thread = (C4JThreadImpl*)argument;
    OSSetThreadName(thread->m_thread, thread->m_name);
    int result = thread->m_entry(thread->m_argument);
    thread->m_exitCode = (uint32_t)result;
    if (!thread->m_deleteOnExit)
    {
        C4JEventImpl* completionEvent = thread->m_completionEvent;
        thread->m_completionEvent = NULL;
        thread->m_running = false;
        completionEvent->Set();
        delete completionEvent;
    }
    else
        delete thread;
    return result;
}

C4JThreadImpl::C4JThreadImpl(int (*entry)(void*), void* argument, const char* name, int stackSize)
    : m_argument(argument), m_entry(entry), m_stackSize(stackSize), m_processor(-1), m_running(false), m_started(false), m_exitCode(259), m_startTime(0), m_completionEvent(new C4JEventImpl(C4JEvent::Mode1)), m_priority(16)
{
    if (m_stackSize == 0)
        m_stackSize = 0x20000;
    else if (m_stackSize < 0x4000)
        m_stackSize = 0x4000;
    sprintf(m_name, "(4J) %s", name);
    m_stack = MEMAllocFromDefaultHeap((unsigned int)m_stackSize);
    m_thread = (OSThread*)MEMAllocFromDefaultHeapEx(0x6a0, 8);
    union
    {
        int (*evidence)(int, const char**);
        int (*source)(int, void*);
    } bridgeEntry;
    bridgeEntry.source = entryPoint;
    OSCreateThread4J(m_thread, bridgeEntry.source, 0, this, (char*)m_stack + m_stackSize, (unsigned int)m_stackSize, 16, 15);
    OSSetThreadRunQuantum(m_thread, 8000);
    EnterCriticalSection(&s_threadSection);
    s_threads.push_back(this);
    LeaveCriticalSection(&s_threadSection);
    m_deleteOnExit = false;
}

C4JThreadImpl::C4JThreadImpl(const char* name)
    : m_argument(NULL), m_entry(NULL), m_stackSize(0), m_processor(1), m_stack(NULL), m_running(true), m_started(true), m_startTime(System::processTimeInMilliSecs()), m_completionEvent(NULL), m_thread((OSThread*)OSGetCurrentThread())
{
    sprintf(m_name, "(4J) %s", name);
    InitializeCriticalSection(&s_threadSection);
    EnterCriticalSection(&s_threadSection);
    s_threads.push_back(this);
    LeaveCriticalSection(&s_threadSection);
}

C4JThreadImpl::~C4JThreadImpl()
{
    if (m_completionEvent != NULL)
    {
        delete m_completionEvent;
        m_completionEvent = NULL;
    }
    EnterCriticalSection(&s_threadSection);
    std::vector<C4JThreadImpl*>::iterator it;
    for (it = s_threads.begin(); it != s_threads.end(); ++it)
    {
        if (*it == this)
        {
            s_threads.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&s_threadSection);
}

void C4JThreadImpl::Run()
{
    m_startTime = System::processTimeInMilliSecs();
    m_started = true;
    m_running = true;
    OSResumeThread(m_thread);
}

bool C4JThreadImpl::isRunning() { return m_running; }
bool C4JThreadImpl::hasStarted() { return m_started; }

void C4JThreadImpl::SetProcessor(int processor)
{
    m_processor = processor;
    if ((unsigned int)processor < 3)
        OSSetThreadAffinity(m_thread, s_processorAffinity[processor]);
    else
        OSSetThreadAffinity(m_thread, 7);
}

uint32_t C4JThreadImpl::GetProcessor() { return m_processor; }

void C4JThreadImpl::SetPriority(int priority)
{
    if ((unsigned int)priority > 2 && (unsigned int)priority < 0xfffffffeu)
    {
        m_priority = 16;
        OSSetThreadPriority(m_thread, 16);
    }
    else
    {
        m_priority = s_priorities[priority];
        OSSetThreadPriority(m_thread, m_priority);
    }
}

uint32_t C4JThreadImpl::WaitForCompletion(int milliseconds)
{
    return m_completionEvent == NULL ? 0 : m_completionEvent->WaitForSignal(milliseconds);
}

uint32_t C4JThreadImpl::GetExitCode() { return m_exitCode; }
const char* C4JThreadImpl::getName() { return m_name; }
void C4JThreadImpl::SetDeleteOnExit(bool deleteOnExit) { m_deleteOnExit = deleteOnExit; }

void C4JThreadImpl::SetMainThreadID()
{
    s_mainThread.m_thread = OSGetCurrentThread();
    OSSetThreadRunQuantum(s_mainThread.m_thread, 8000);
}

void C4JThreadImpl::Sleep(int milliseconds)
{
    OSSleepTicks(OSMillisecondsToTicks(milliseconds));
}

void SetSpecialThreadIDForFS_MilesStreamThread()
{
    s_specialThreadIDForFS_MilesStreamThread = 0x40000000u;
}

static void insertThreads(
    C4JThreadVector& vector, C4JThreadVectorIterator position, size_t count, C4JThreadImpl* const& value)
{
    vector.insert(position, count, value);
}

static C4JThreadVectorIterator insertThread(
    C4JThreadVector& vector, C4JThreadVectorIterator position, C4JThreadImpl* const& value)
{
    return vector.insert(position, value);
}

static C4JThreadVectorIterator eraseThread(
    C4JThreadVector& vector, C4JThreadVectorIterator position)
{
    return vector.erase(position);
}
