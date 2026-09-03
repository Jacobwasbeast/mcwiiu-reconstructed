#ifndef SOURCE_PLATFORM_WIIU_C4JEVENT_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENT_HPP

class C4JEvent
{
public:
    enum EMode
    {
        Mode0 = 0,
        Mode1 = 1
    };

    virtual ~C4JEvent();
    virtual void Set() = 0;
    virtual void Clear() = 0;
    virtual unsigned int WaitForSignal(int timeout) = 0;
};

#endif
