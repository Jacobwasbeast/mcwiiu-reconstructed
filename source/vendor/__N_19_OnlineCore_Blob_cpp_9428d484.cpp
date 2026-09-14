#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"
#include <string.h>

#include "nn/nex/StringConversion.hpp"

namespace {
static unsigned int streamByte(const nn::nex::ByteStream*, unsigned int) { return 0; }
static unsigned int streamLength(const nn::nex::ByteStream*) { return 0; }
static bool rendezvousState() { return false; }
static void closeEndpoint(nn::nex::EndPoint*) {}
static void notifyDisconnect(unsigned int, bool) {}
static uint32_t packetSignature(const nn::nex::Packet*, nn::nex::Stream::Type, const unsigned char*, int, bool) { return 0; }
}

void __N_19_OnlineCore_Blob_cpp_9428d484::_Copy(const wchar_t* first, unsigned int firstLength, const wchar_t* second, unsigned int secondLength)
{
    if (m_data == 0) return;
    if (first != 0) memcpy(m_data, first, firstLength * sizeof(wchar_t));
    if (second != 0) memcpy(m_data + firstLength, second, secondLength * sizeof(wchar_t));
    m_data[firstLength + secondLength] = 0;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::UnicodeToUtf8(const wchar_t* input, char* output, unsigned int capacity)
{
    return nn::nex::StringConversion::TToUtf8(input, output, capacity);
}

int __N_19_OnlineCore_Blob_cpp_9428d484::Utf8ToUnicode(const char* input, wchar_t* output, unsigned int capacity)
{
    if (input == 0 || output == 0) return -1;
    unsigned int* destination = reinterpret_cast<unsigned int*>(output);
    if (*input == 0 || capacity == 1) { *destination = 0; return 1; }
    if (capacity == 0) capacity = 0xffffffffu;

    unsigned int converted = 0;
    while (converted + 1 < capacity)
    {
        unsigned int lead = static_cast<unsigned int>(static_cast<unsigned char>(*input));
        if (static_cast<signed char>(lead) == 0)
        {
            destination[converted] = 0;
            return static_cast<int>(converted + 1);
        }

        unsigned int value;
        unsigned int width;
        if (lead < 0x80)
        {
            value = static_cast<unsigned int>(static_cast<signed char>(lead));
            width = 1;
        }
        else if (lead - 0xc2 < 0x1e)
        {
            value = lead * 0x40 + static_cast<unsigned char>(input[1]) - 0x3080;
            width = 2;
        }
        else if (lead - 0xe0 < 0x10)
        {
            value = (lead << 12) + (static_cast<unsigned char>(input[1]) << 6) + static_cast<unsigned char>(input[2]) - 0xe2080;
            width = 3;
        }
        else
        {
              destination[0] = 0;
            return -1;
        }

        destination[converted] = value & 0xffffu;
        ++converted;
        input += width;
        if (static_cast<signed char>(*input) == 0)
        {
            destination[converted] = 0;
            return static_cast<int>(converted + 1);
        }
    }

    destination[converted] = 0;
    return static_cast<int>(converted + 1);
}

int __N_19_OnlineCore_Blob_cpp_9428d484::GetNeededUtf8BufferSize(const wchar_t* input)
{
    return nn::nex::StringConversion::GetTToUtf8BufferSize(input);
}

uint64_t __N_19_OnlineCore_Blob_cpp_9428d484::IsRendezVousRunning() { return rendezvousState() ? 1 : 0; }
uint32_t __N_19_OnlineCore_Blob_cpp_9428d484::PRUDPVersionCheck(const nn::nex::ByteStream* stream)
{
    if (streamByte(stream, 0) != 0xea && streamByte(stream, 1) != 0xd0) return 0;
    unsigned int version = streamByte(stream, 2); if (version == 0) return 0xffffff00u; return version < 2 ? version : (uint32_t)(-version);
}
bool __N_19_OnlineCore_Blob_cpp_9428d484::PRUDPUnpackCheck(const nn::nex::ByteStream* stream, unsigned int* value) { if (value != 0) *value = 0; return streamLength(stream) > 9; }
uint32_t __N_19_OnlineCore_Blob_cpp_9428d484::RelayProtocolCheck(const nn::nex::ByteStream* stream) { return streamByte(stream, 0) == 0xf5 && streamByte(stream, 1) == 0xd0; }
uint64_t __N_19_OnlineCore_Blob_cpp_9428d484::DisconnectCallback(nn::nex::EndPoint* endpoint, nn::nex::qResult result, const nn::nex::UserContext* context)
{
    bool successful = result.returnCode == 1;
    unsigned int contextId = context == 0 ? 0 : result.errorFile;
    notifyDisconnect(contextId, successful);
    if (endpoint != 0) closeEndpoint(endpoint);
    return successful ? 1 : 0;
}
uint32_t __N_19_OnlineCore_Blob_cpp_9428d484::CalcDataPacketSignature(const nn::nex::Packet* packet, nn::nex::Stream::Type type, const unsigned char* data, int length, bool secure) { if (data == 0) return 0x12345678; return packetSignature(packet, type, data, length, secure); }
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvFromCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const nn::nex::SocketDriver::InetAddress*, const unsigned int*) {}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendToCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const nn::nex::SocketDriver::InetAddress&, const unsigned int*) {}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const unsigned int*) {}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const unsigned int*) {}
