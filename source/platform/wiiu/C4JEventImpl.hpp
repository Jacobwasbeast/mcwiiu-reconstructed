#ifndef SOURCE_PLATFORM_WIIU_C4JEVENTIMPL_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENTIMPL_HPP

struct OSThread;

struct OSThreadQueue
{
    OSThread* pHead;
    OSThread* pTail;
    void* pParent;
    unsigned int uiReserved;
};

struct OSEvent
{
    unsigned int uiTag;
    const char* pName;
    unsigned int uiReserved;
    OSThreadQueue waitQueue;
    unsigned int uiValue;
    int iMode;
};

class C4JEvent
{
public:
    enum EMode
    {
        Mode0 = 0,
        Mode1 = 1
    };
};

class C4JEventImpl
{
public:
    C4JEventImpl(C4JEvent::EMode mode);
    virtual ~C4JEventImpl();
    virtual void Set();
    virtual void Clear();
    virtual unsigned int WaitForSignal(int timeout);

private:
    C4JEvent::EMode m_mode;
    int m_waitCount;
    OSEvent m_event;
};

#endif
