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
static OSMutex s_threadMutex;
static OSMutex s_createMutex;
static OSThread* s_mainThread = NULL;
static uint32_t s_specialThreadIDForFS_MilesStreamThread = 0;
static const unsigned int s_processorAffinity[3] = { 0x01020400u, 0x1211100Fu, 0x0E000000u };
static const unsigned char s_priorities[] = { 0x10, 0x0F, 0x0E, 0x00 };

void SetThreadName(unsigned int, const char* name)
{
    OSSetThreadName(OSGetCurrentThread(), name);
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

struct ThreadMutexInitializer
{
    ThreadMutexInitializer()
    {
        OSInitMutex(&s_threadMutex);
        OSInitMutex(&s_createMutex);
    }
};

static void InitializeThreadMutexes()
{
    static ThreadMutexInitializer initializer;
    (void)initializer;
}

uint32_t C4JThreadImpl::getCurrentThread()
{
    OSThread* currentThread = OSGetCurrentThread();
    InitializeThreadMutexes();
    OSLockMutex(&s_threadMutex);
    std::vector<C4JThreadImpl*>::iterator it;
    for (it = s_threads.begin(); it != s_threads.end(); ++it)
    {
        if ((*it)->m_thread == currentThread)
        {
            C4JThreadImpl* thread = *it;
            OSUnlockMutex(&s_threadMutex);
            return (uint32_t)thread;
        }
    }
    OSUnlockMutex(&s_threadMutex);
    return 0;
}

bool C4JThreadImpl::isMainThread()
{
    OSThread* currentThread = OSGetCurrentThread();
    InitializeThreadMutexes();
    OSLockMutex(&s_threadMutex);
    bool result = currentThread == s_mainThread;
    OSUnlockMutex(&s_threadMutex);
    return result;
}

const char* C4JThreadImpl::getCurrentThreadName()
{
    return ((C4JThreadImpl*)getCurrentThread())->getName();
}

void C4JThreadImpl::threadDeallocator(void*, void* stack)
{
    MEMFreeToDefaultHeap(stack);
}

static void ThreadDeallocatorAdapter(OSThread* thread, void* stack)
{
    C4JThreadImpl::threadDeallocator(thread, stack);
}

extern "C" BOOL OSCreateThreadEvidence(OSThread*, int (*)(int, const char**), int, char*, void*, uint32_t, int32_t, OSThreadAttributes) __asm__("OSCreateThread");
extern "C" OSThreadDeallocatorFn OSSetThreadDeallocatorEvidence(OSThread*, void (*)(OSThread*, void*)) __asm__("OSSetThreadDeallocator");

uint32_t C4JThreadImpl::OSCreateThread4J(OSThread* thread, int (*entry)(int, void*), int argumentCount, void* argument, void* stack, unsigned int stackSize, int priority, unsigned short attributes)
{
    union
    {
        int (*source)(int, void*);
        int (*evidence)(int, const char**);
    } evidenceEntry = { entry };
    union
    {
        void* pointer;
        char* argv;
    } evidenceArgument = { argument };

    InitializeThreadMutexes();
    OSLockMutex(&s_createMutex);
    uint32_t result = OSCreateThreadEvidence(thread, evidenceEntry.evidence, argumentCount, evidenceArgument.argv, stack, stackSize, priority, attributes);
    OSSetThreadDeallocatorEvidence(thread, ThreadDeallocatorAdapter);
    OSUnlockMutex(&s_createMutex);
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
    : m_argument(argument), m_entry(entry), m_stackSize(stackSize), m_processor(-1), m_stack(NULL), m_running(false), m_started(false), m_exitCode(259), m_startTime(0), m_deleteOnExit(false), m_completionEvent(new C4JEventImpl(C4JEvent::Mode1)), m_priority(16), m_thread(NULL)
{
    if (m_stackSize == 0)
        m_stackSize = 0x20000;
    else if (m_stackSize < 0x4000)
        m_stackSize = 0x4000;
    sprintf(m_name, "(4J) %s", name);
    m_stack = MEMAllocFromDefaultHeapEx((unsigned int)m_stackSize, 8);
    m_thread = (OSThread*)MEMAllocFromDefaultHeapEx(0x6a0, 8);
    union
    {
        int (*evidence)(int, const char**);
        int (*source)(int, void*);
    } bridgeEntry;
    bridgeEntry.source = entryPoint;
    OSCreateThread4J(m_thread, bridgeEntry.source, 0, this, (char*)m_stack + m_stackSize, (unsigned int)m_stackSize, 16, 15);
    OSSetThreadRunQuantum(m_thread, 8000);
    OSLockMutex(&s_threadMutex);
    s_threads.push_back(this);
    OSUnlockMutex(&s_threadMutex);
    m_deleteOnExit = false;
}

C4JThreadImpl::C4JThreadImpl(const char* name)
    : m_argument(NULL), m_entry(NULL), m_stackSize(0), m_processor(1), m_stack(NULL), m_running(true), m_started(true), m_exitCode(1), m_startTime(System::processTimeInMilliSecs()), m_deleteOnExit(false), m_completionEvent(NULL), m_priority(0), m_thread((OSThread*)OSGetCurrentThread())
{
    sprintf(m_name, "(4J) %s", name);
    InitializeThreadMutexes();
    OSLockMutex(&s_threadMutex);
    s_threads.push_back(this);
    OSUnlockMutex(&s_threadMutex);
}

C4JThreadImpl::~C4JThreadImpl()
{
    delete m_completionEvent;
    m_completionEvent = NULL;
    InitializeThreadMutexes();
    OSLockMutex(&s_threadMutex);
    std::vector<C4JThreadImpl*>::iterator it;
    for (it = s_threads.begin(); it != s_threads.end(); ++it)
    {
        if (*it == this)
        {
            s_threads.erase(it);
            break;
        }
    }
    OSUnlockMutex(&s_threadMutex);
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
    if (processor < 3)
        OSSetThreadAffinity(m_thread, s_processorAffinity[processor]);
    else
        OSSetThreadAffinity(m_thread, 7);
}

uint32_t C4JThreadImpl::GetProcessor() { return m_processor; }

void C4JThreadImpl::SetPriority(int priority)
{
    if (priority > 2 && priority < 0xfffffffeu)
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
    InitializeThreadMutexes();
    OSLockMutex(&s_threadMutex);
    s_mainThread = OSGetCurrentThread();
    OSSetThreadRunQuantum(s_mainThread, 8000);
    OSUnlockMutex(&s_threadMutex);
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
