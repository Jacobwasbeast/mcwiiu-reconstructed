#ifndef SOURCE_VENDOR_N_19_ONLINECORE_BLOB_CPP_9428D484_HPP
#define SOURCE_VENDOR_N_19_ONLINECORE_BLOB_CPP_9428D484_HPP

#include <stdint.h>
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{
class ByteStream;
class EndPoint;
class Packet;
class UserContext;
class SocketDriver
{
public:
    enum _Result
    {
    };
    class InetAddress
    {
    };
};
namespace Stream
{
enum Type
{
    TypeUnknown = 0
};
}
}
}

namespace __N_19_OnlineCore_Blob_cpp_9428d484
{
    nn::nex::String _Copy(const wchar_t*, unsigned int, const wchar_t*, unsigned int);
    int UnicodeToUtf8(const wchar_t*, char*, unsigned int);
    int Utf8ToUnicode(const char*, wchar_t*, unsigned int);
    int GetNeededUtf8BufferSize(const wchar_t*);
    uint64_t IsRendezVousRunning();
    uint32_t PRUDPVersionCheck(const nn::nex::ByteStream*);
    bool PRUDPUnpackCheck(const nn::nex::ByteStream*, unsigned int*);
    uint32_t RelayProtocolCheck(const nn::nex::ByteStream*);
    uint64_t DisconnectCallback(nn::nex::EndPoint*, nn::nex::qResult, const nn::nex::UserContext*);
    uint32_t CalcDataPacketSignature(const nn::nex::Packet*, nn::nex::Stream::Type, const unsigned char*, int,
                                     bool);
    void DefaultRecvFromCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int,
                                 const nn::nex::SocketDriver::InetAddress*, const unsigned int*);
    void DefaultSendToCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int,
                               const nn::nex::SocketDriver::InetAddress&, const unsigned int*);
    void DefaultRecvCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int,
                             const unsigned int*);
    void DefaultSendCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int,
                             const unsigned int*);
    int findReturnCode(int*, int);
    int GetNeededWideCharBufferSize(const char*);
    int Latin1ToUtf8(const char*, char*, unsigned int);
    void StackCheck(bool);
    int GetChar8ToUtf8BufferSize(const char*);
    int Utf8ToLatin1(const char*, char*, unsigned int);
    bool IsRunningAsBackEnd();
}

#endif
