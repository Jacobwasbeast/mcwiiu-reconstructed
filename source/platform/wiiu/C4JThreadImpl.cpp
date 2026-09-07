#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/System.hpp"
#include <stdio.h>
#include <vector>

extern "C" uint32_t OSGetCurrentThread();
extern "C" void OSSetThreadName(OSThread*, const char*);
extern "C" uint32_t OSCreateThread(OSThread*, int (*)(int, void*), int, void*, void*, unsigned int, unsigned int, unsigned short);
extern "C" void OSSetThreadDeallocator(OSThread*, void (*)(void*, void*));
extern "C" void OSSetThreadRunQuantum(OSThread*, unsigned int);
extern "C" int OSResumeThread(OSThread*);
extern "C" void OSSetThreadAffinity(OSThread*, unsigned int);
extern "C" void OSSetThreadPriority(OSThread*, unsigned int);
extern "C" void* MEMAllocFromDefaultHeapEx(unsigned int, int);
extern "C" void MEMFreeToDefaultHeap(void*);
extern "C" void Sleep(unsigned int);

static std::vector<C4JThreadImpl*> s_threads;
static OSFastMutex s_threadMutex;
static OSFastMutex s_createMutex;
static bool s_createMutexInitialized = false;
static OSThread* s_mainThread = NULL;
static __thread uint32_t s_specialThreadIDForFS_MilesStreamThread = 0;
static const unsigned int s_processorAffinity[3] = { 0x01020400u, 0x1211100Fu, 0x0E000000u };
static const unsigned char s_priorities[] = { 0x10, 0x0F, 0x0E, 0x00 };

typedef std::vector<C4JThreadImpl*> C4JThreadVector;
typedef C4JThreadVector::iterator C4JThreadVectorIterator;

uint32_t C4JThreadImpl::getCurrentThread()
{
    uint32_t currentThread = OSGetCurrentThread();
    EnterCriticalSection(&s_threadMutex);
    std::vector<C4JThreadImpl*>::iterator it;
    for (it = s_threads.begin(); it != s_threads.end(); ++it)
    {
        if ((uint32_t)(*it)->m_thread == currentThread)
        {
            C4JThreadImpl* thread = *it;
            LeaveCriticalSection(&s_threadMutex);
            return (uint32_t)thread;
        }
    }
    LeaveCriticalSection(&s_threadMutex);
    return 0;
}

bool C4JThreadImpl::isMainThread()
{
    return (OSThread*)getCurrentThread() == s_mainThread;
}

const char* C4JThreadImpl::getCurrentThreadName()
{
    return ((C4JThreadImpl*)getCurrentThread())->getName();
}

void C4JThreadImpl::threadDeallocator(void*, void* stack)
{
    MEMFreeToDefaultHeap(stack);
}

uint32_t C4JThreadImpl::OSCreateThread4J(OSThread* thread, int (*entry)(int, void*), int argumentCount, void* argument, void* stack, unsigned int stackSize, unsigned int priority, unsigned short attributes)
{
    if (!s_createMutexInitialized)
    {
        InitializeCriticalSection(&s_createMutex);
        s_createMutexInitialized = true;
    }
    EnterCriticalSection(&s_createMutex);
    uint32_t result = OSCreateThread(thread, entry, argumentCount, argument, stack, stackSize, priority, attributes);
    OSSetThreadDeallocator(thread, threadDeallocator);
    LeaveCriticalSection(&s_createMutex);
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
        delete thread->m_completionEvent;
        thread->m_completionEvent = NULL;
        thread->m_running = 0;
        delete thread;
    }
    else
        thread->m_running = 0;
    return result;
}

C4JThreadImpl::C4JThreadImpl(int (*entry)(void*), void* argument, const char* name, int stackSize)
    : m_argument(argument), m_entry(entry), m_stackSize(stackSize), m_processor(-1), m_stack(NULL), m_running(0), m_started(0), m_exitCode(259), m_startTime(0), m_deleteOnExit(false), m_completionEvent(new C4JEventImpl(C4JEvent::Mode1)), m_priority(16), m_thread(NULL)
{
    if (m_stackSize == 0)
        m_stackSize = 0x20000;
    else if (m_stackSize < 0x4000)
        m_stackSize = 0x4000;
    sprintf(m_name, "(4J) %s", name);
    m_stack = MEMAllocFromDefaultHeapEx((unsigned int)m_stackSize, 8);
    m_thread = (OSThread*)MEMAllocFromDefaultHeapEx(0x6a0, 8);
    OSCreateThread4J(m_thread, entryPoint, 0, this, (char*)m_stack + m_stackSize, (unsigned int)m_stackSize, 16, 15);
    OSSetThreadRunQuantum(m_thread, 8000);
    EnterCriticalSection(&s_threadMutex);
    s_threads.push_back(this);
    LeaveCriticalSection(&s_threadMutex);
    m_deleteOnExit = false;
}

C4JThreadImpl::C4JThreadImpl(const char* name)
    : m_argument(NULL), m_entry(NULL), m_stackSize(0), m_processor(1), m_stack(NULL), m_running(1), m_started(1), m_exitCode(1), m_startTime(System::processTimeInMilliSecs()), m_deleteOnExit(false), m_completionEvent(NULL), m_priority(0), m_thread((OSThread*)OSGetCurrentThread())
{
    sprintf(m_name, "(4J) %s", name);
    InitializeCriticalSection(&s_threadMutex);
    EnterCriticalSection(&s_threadMutex);
    s_threads.push_back(this);
    LeaveCriticalSection(&s_threadMutex);
}

C4JThreadImpl::~C4JThreadImpl()
{
    delete m_completionEvent;
    m_completionEvent = NULL;
    EnterCriticalSection(&s_threadMutex);
    std::vector<C4JThreadImpl*>::iterator it;
    for (it = s_threads.begin(); it != s_threads.end(); ++it)
    {
        if (*it == this)
        {
            s_threads.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&s_threadMutex);
}

void C4JThreadImpl::Run()
{
    m_startTime = System::processTimeInMilliSecs();
    m_started = 1;
    m_running = 1;
    OSResumeThread(m_thread);
}

uint8_t C4JThreadImpl::isRunning() { return m_running; }
uint8_t C4JThreadImpl::hasStarted() { return m_started; }

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
    s_mainThread = (OSThread*)OSGetCurrentThread();
    OSSetThreadRunQuantum(s_mainThread, 8000);
}

void C4JThreadImpl::Sleep(int milliseconds)
{
    ::Sleep((unsigned int)milliseconds);
}

void SetSpecialThreadIDForFS_MilesStreamThread__Fv()
{
    s_specialThreadIDForFS_MilesStreamThread = 0x40000000u;
}

void __CPR229___Insert_n__Q2_3std74vector__tm__60_P13C4JJThreadImplQ2_3std34allocator__tm__17_PJ36JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyQ4_3stdJ139JJ166J9size_typeRCZ1Z_v(
    C4JThreadVector& vector, C4JThreadVectorIterator position, size_t count, C4JThreadImpl* const& value)
{
    vector.insert(position, count, value);
}

C4JThreadVectorIterator __CPR252__insert__Q2_3std74vector__tm__60_P13C4JJThreadImplQ2_3std34allocator__tm__17_PJ33JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_AltyRCZ1Z_Q2_3stdJ99J(
    C4JThreadVector& vector, C4JThreadVectorIterator position, C4JThreadImpl* const& value)
{
    return vector.insert(position, value);
}

C4JThreadVectorIterator __CPR246__erase__Q2_3std74vector__tm__60_P13C4JJThreadImplQ2_3std34allocator__tm__17_PJ32JFQ2_3std68_Vector_iterator__tm__44_Z1ZQ3_3std25_Vector_val__tm__7_Z1ZZ2Z5_Alty_Q2_3stdJ98J(
    C4JThreadVector& vector, C4JThreadVectorIterator position)
{
    return vector.erase(position);
}
