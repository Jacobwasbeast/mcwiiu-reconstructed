#ifndef LEGACY_CONSOLE_WIIU_NEX_H
#define LEGACY_CONSOLE_WIIU_NEX_H

#include <stdint.h>
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/UserContext.hpp"
#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"

namespace nn
{
namespace nex
{
class EndPoint;
class String;

class InetAddress
{
public:
    virtual ~InetAddress();
    InetAddress();
    InetAddress(const InetAddress& other);
    InetAddress(unsigned int address, unsigned short port);
    InetAddress(const wchar_t* address, unsigned short port);
    void Init();
    InetAddress& operator=(const InetAddress& other);
    unsigned int GetAddress() const;
    unsigned short GetPortNumber() const;
    void SetAddress(unsigned int address);
    void SetPortNumber(unsigned short port);
    void SetLocalHost();
    static unsigned int String2Address(const char* address);
    unsigned int SetAddress(const wchar_t* address);
    unsigned int GetAddress(wchar_t* buffer, unsigned int size) const;
    unsigned int GetPortNumber(wchar_t* buffer, unsigned int size) const;
    String GetPortNumberStr() const;
    void ToStr(wchar_t* buffer) const;
    static void EnableAutoLookup(bool enable);
    InetAddress(void* address, unsigned int size);
    void SetNetworkAddress(unsigned int address);
    String GetAddressStr() const;
    void SetNetworkPortNumber(unsigned short port);
    bool IsLocalHost() const;
    String ToStr() const;
    virtual void Trace(unsigned long long value) const;
private:
    unsigned char m_data[16];
};

class JobConnectEndPoint : public Job
{
public:
    void ReportConnectResult(qResult result);
};

class CallbackRoot
{
public:
    virtual ~CallbackRoot();
    virtual void Call();
};

class NotificationEvent
{
public:
    unsigned int GetType() const;
    unsigned int GetParam2() const;
};

class NotificationEventHandler
{
public:
    virtual ~NotificationEventHandler();
    virtual void ProcessNotificationEvent(const NotificationEvent& event);
};

class StringStream
{
public:
    StringStream();
    virtual ~StringStream();
    int GetLength() const;
    void FreeBuffer(wchar_t* buffer);
    void ResizeBuffer(unsigned int size);
    void TestFreeRoom(unsigned int size);
    int __ls(const wchar_t* value);
    void AddBaseIfRequired();
    StringStream* StreamNumber(int value);
    void FreeBuffer();
    void __ls(bool value);
    StringStream* StreamNumber(unsigned int value);
    StringStream* StreamNumber(unsigned char value);
    void __ls(const void* value);
    void Clear();
    StringStream* __ls(long long value);
    StringStream* __ls(unsigned long long value);
    StringStream* __ls(const StringStream& value);
    StringStream* __ls(double value);
    StringStream* __ls(float value);
    void BytesDump(const unsigned char* data, unsigned int size);
};

class InetAddress;
class Time;
class Socket;
class SignatureBytes;
class DateTime;
class Variant;
class HTTPURL;
class StationURL;
class UserMessage;
class MessageRecipient;
class IOCompletionContext;

StringStream& __ls(StringStream& stream, const String& value);
StringStream& __ls(StringStream& stream, const InetAddress& value);
StringStream& __ls(StringStream& stream, const qResult& value);
StringStream& __ls(StringStream& stream, const Time& value);
StringStream& __ls(StringStream& stream, Socket& value);
StringStream& __ls(StringStream& stream, const IOCompletionContext& value);
StringStream& __ls(StringStream& stream, const SignatureBytes& value);
StringStream& __ls(StringStream& stream, const DateTime& value);
StringStream& __ls(StringStream& stream, const Variant& value);
StringStream& __ls(StringStream& stream, const HTTPURL& value);
StringStream& __ls(StringStream& stream, const StationURL& value);
StringStream& __ls(StringStream& stream, const NotificationEvent& event);
StringStream& __ls(StringStream& stream, const UserMessage& value);
StringStream& __ls(StringStream& stream, const MessageRecipient& value);
StringStream& __ls(StringStream& stream, const StringStream& value);
StringStream& hex(StringStream& stream);
StringStream& dec(StringStream& stream);
StringStream& showbase(StringStream& stream);
StringStream& zerohex(StringStream& stream);
StringStream& noshowbase(StringStream& stream);
StringStream& endl(StringStream& stream);
StringStream& boolalpha(StringStream& stream);
StringStream& noboolalpha(StringStream& stream);

class BadEvents
{
public:
    enum Event
    {
        SYSTEM_CHECK_FAILURE = 0
    };

    static void Signal(Event event);
};
}
}

#endif
