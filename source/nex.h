#ifndef LEGACY_CONSOLE_WIIU_NEX_H
#define LEGACY_CONSOLE_WIIU_NEX_H

#include <stdint.h>
#include "nn/nex/CallbackRoot.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/JobConnectEndPoint.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventHandler.hpp"
#include "nn/nex/StringStream.hpp"
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
StringStream& __ls(StringStream& stream, const UserMessage& value);
StringStream& __ls(StringStream& stream, const MessageRecipient& value);
StringStream& __ls(StringStream& stream, const StringStream& value);
StringStream& hex(StringStream& stream);
StringStream& dec(StringStream& stream);
StringStream& showbase(StringStream& stream);
StringStream& noshowbase(StringStream& stream);
StringStream& endl(StringStream& stream);
StringStream& boolalpha(StringStream& stream);
StringStream& noboolalpha(StringStream& stream);

}
}

#include "nn/nex/BadEvents.hpp"

#endif
