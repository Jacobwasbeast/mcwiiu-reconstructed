#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"
#include <string.h>

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
    unsigned int left = capacity == 0 ? 0 : capacity - 1;
    int written = 0;
    if (output == 0) return 0;
    while (input != 0 && *input != 0 && left != 0) {
        unsigned int c = (unsigned int)(*input++);
        if (c < 0x80) { *output++ = (char)c; --left; ++written; }
        else if (c < 0x800 && left >= 2) { *output++ = (char)(0xc0 | (c >> 6)); *output++ = (char)(0x80 | (c & 0x3f)); left -= 2; written += 2; }
        else if (c < 0x10000 && left >= 3) { *output++ = (char)(0xe0 | (c >> 12)); *output++ = (char)(0x80 | ((c >> 6) & 0x3f)); *output++ = (char)(0x80 | (c & 0x3f)); left -= 3; written += 3; }
        else break;
    }
    *output = 0;
    return written;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::Utf8ToUnicode(const char* input, wchar_t* output, unsigned int capacity)
{
    unsigned int left = capacity == 0 ? 0 : capacity - 1;
    int count = 0;
    if (input == 0 || output == 0) return -1;
    while (*input != 0 && left != 0) {
        unsigned int c = (unsigned char)*input++;
        if (c < 0x80) *output++ = (wchar_t)c;
        else if (c >= 0xc2 && c <= 0xdf) { if (*input == 0) return -1; *output++ = (wchar_t)(((c & 0x1f) << 6) | ((unsigned char)*input++ & 0x3f)); }
        else if (c >= 0xe0 && c <= 0xef) { if (input[0] == 0 || input[1] == 0) return -1; *output++ = (wchar_t)(((c & 0xf) << 12) | (((unsigned char)input[0] & 0x3f) << 6) | ((unsigned char)input[1] & 0x3f)); input += 2; }
        else return -1;
        --left; ++count;
    }
    *output = 0;
    return count + 1;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::GetNeededUtf8BufferSize(const wchar_t* input)
{
    int size = 1;
    while (input != 0 && *input != 0) { unsigned int c = (unsigned int)*input++; size += c < 0x80 ? 1 : (c < 0x800 ? 2 : (c < 0x10000 ? 3 : 0)); }
    return size;
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
int __N_19_OnlineCore_Blob_cpp_9428d484::findReturnCode(int* values, int count) { int i; if (values == 0 || count <= 0) return 0; for (i = 0; i < count; ++i) if (values[i] != 0) return values[i]; return 0; }
int __N_19_OnlineCore_Blob_cpp_9428d484::GetNeededWideCharBufferSize(const char* input) { int size = 1; if (input != 0) while (*input++ != 0) ++size; return size; }
int __N_19_OnlineCore_Blob_cpp_9428d484::Latin1ToUtf8(const char* input, char* output, unsigned int capacity) { unsigned int left = capacity == 0 ? 0 : capacity - 1; int written = 0; if (input == 0 || output == 0) return 0; while (*input != 0 && left != 0) { unsigned int c = (unsigned char)*input++; if (c < 0x80) { *output++ = (char)c; --left; ++written; } else if (left >= 2) { *output++ = (char)(0xc0 | (c >> 6)); *output++ = (char)(0x80 | (c & 0x3f)); left -= 2; written += 2; } else break; } *output = 0; return written; }
void __N_19_OnlineCore_Blob_cpp_9428d484::StackCheck(bool enabled) { volatile unsigned int marker = enabled ? 0xa5a5a5a5u : 0; if (marker == 0xffffffffu) StackCheck(false); }
int __N_19_OnlineCore_Blob_cpp_9428d484::GetChar8ToUtf8BufferSize(const char* input) { int size = 1; if (input != 0) while (*input != 0) { size += ((unsigned char)*input++ < 0x80) ? 1 : 2; } return size; }
int __N_19_OnlineCore_Blob_cpp_9428d484::Utf8ToLatin1(const char* input, char* output, unsigned int capacity) { if (input == 0 || output == 0 || capacity == 0) return 0; strncpy(output, input, capacity - 1); output[capacity - 1] = 0; return (int)strlen(output); }
bool __N_19_OnlineCore_Blob_cpp_9428d484::IsRunningAsBackEnd() { return m_field04 != 0 && m_field08 == 0; }
