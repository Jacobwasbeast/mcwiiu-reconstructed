#ifndef SOURCE_VENDOR_N_19_ONLINECORE_BLOB_CPP_9428D484_HPP
#define SOURCE_VENDOR_N_19_ONLINECORE_BLOB_CPP_9428D484_HPP

#include <stdint.h>
#include "nn/nex/qResult.hpp"

namespace nn { namespace nex {
class ByteStream;
class EndPoint;
class Packet;
class UserContext;
class SocketDriver { public: typedef int _Result; class InetAddress {}; };
namespace Stream { enum Type { TypeUnknown = 0 }; }
}}

class __N_19_OnlineCore_Blob_cpp_9428d484
{
public:
    wchar_t* m_data;
    uint32_t m_field04;
    uint32_t m_field08;
    uint32_t m_field0c;
    uint32_t m_field10;
    uint32_t m_field14;
    uint32_t m_field18;
    uint32_t m_field1c;
    uint32_t m_field20;
    uint32_t m_field24;
    uint32_t m_field28;
    uint32_t m_field2c;
    uint32_t m_field30;
    uint32_t m_field34;
    uint32_t m_field38;
    uint32_t m_field3c;
    uint32_t m_field40;
    uint32_t m_field44;
    uint32_t m_field48;
    uint32_t m_field4c;
    uint32_t m_field50;
    uint32_t m_field54;
    uint32_t m_field58;
    uint32_t m_field5c;
    uint32_t m_field60;
    uint32_t m_field64;
    uint32_t m_field68;
    uint32_t m_field6c;
    uint32_t m_field70;
    uint32_t m_field74;
    uint32_t m_field78;
    uint32_t m_field7c;
    uint32_t m_field80;
    uint32_t m_field84;
    uint32_t m_field88;
    uint32_t m_field8c;
    uint32_t m_field90;
    uint32_t m_field94;
    uint32_t m_field98;
    uint32_t m_field9c;
    uint32_t m_fielda0;
    uint32_t m_fielda4;
    uint32_t m_fielda8;
    uint32_t m_fieldac;
    uint32_t m_fieldb0;
    uint32_t m_fieldb4;
    uint32_t m_fieldb8;
    uint32_t m_fieldbc;
    uint32_t m_fieldc0;
    uint32_t m_fieldc4;
    uint32_t m_fieldc8;
    uint32_t m_fieldcc;
    uint32_t m_fieldd0;
    uint32_t m_fieldd4;
    uint32_t m_fieldd8;
    uint32_t m_fielddc;
    void _Copy(const wchar_t*, unsigned int, const wchar_t*, unsigned int);
    int UnicodeToUtf8(const wchar_t*, char*, unsigned int);
    int Utf8ToUnicode(const char*, wchar_t*, unsigned int);
    int GetNeededUtf8BufferSize(const wchar_t*);
    uint64_t IsRendezVousRunning();
    uint32_t PRUDPVersionCheck(const nn::nex::ByteStream*);
    bool PRUDPUnpackCheck(const nn::nex::ByteStream*, unsigned int*);
    uint32_t RelayProtocolCheck(const nn::nex::ByteStream*);
    uint64_t DisconnectCallback(nn::nex::EndPoint*, nn::nex::qResult, const nn::nex::UserContext*);
    uint32_t CalcDataPacketSignature(const nn::nex::Packet*, nn::nex::Stream::Type, const unsigned char*, int, bool);
    void DefaultRecvFromCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const nn::nex::SocketDriver::InetAddress*, const unsigned int*);
    void DefaultSendToCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const nn::nex::SocketDriver::InetAddress&, const unsigned int*);
    void DefaultRecvCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const unsigned int*);
    void DefaultSendCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const unsigned int*);
    int findReturnCode(int*, int);
    int GetNeededWideCharBufferSize(const char*);
    int Latin1ToUtf8(const char*, char*, unsigned int);
    void StackCheck(bool);
    int GetChar8ToUtf8BufferSize(const char*);
    int Utf8ToLatin1(const char*, char*, unsigned int);
    bool IsRunningAsBackEnd();
};

#endif
