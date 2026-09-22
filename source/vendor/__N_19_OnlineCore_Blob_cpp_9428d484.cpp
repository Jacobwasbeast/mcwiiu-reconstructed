#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"
#include <string.h>

void __N_19_OnlineCore_Blob_cpp_9428d484::_Copy(const wchar_t* first, unsigned int firstLength, const wchar_t* second, unsigned int secondLength)
{
    wcscpy(m_data, first);
    wcscpy(m_data + firstLength, second);
}

int __N_19_OnlineCore_Blob_cpp_9428d484::UnicodeToUtf8(const wchar_t* input, char* output, unsigned int capacity)
{
    int written = 0;
    int remaining = static_cast<int>(capacity) - 1;
    bool invalid = false;
    while (*input != 0 && remaining != 0 && !invalid)
    {
        const unsigned int value = static_cast<unsigned int>(*input++);
        if (value < 0x80)
        {
            *output++ = static_cast<char>(value);
            --remaining;
            ++written;
        }
        else if (value < 0x800)
        {
            *output++ = static_cast<char>((value >> 6) | 0xc0);
            *output++ = static_cast<char>((value & 0x3f) | 0x80);
            remaining -= 2;
            written += 2;
        }
        else if (value < 0x10000)
        {
            *output++ = static_cast<char>((value >> 12) | 0xe0);
            *output++ = static_cast<char>(((value >> 6) & 0x3f) | 0x80);
            *output++ = static_cast<char>((value & 0x3f) | 0x80);
            remaining -= 3;
            written += 3;
        }
        else
            invalid = true;
    }
    *output = 0;
    return written;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::Utf8ToUnicode(const char* input, wchar_t* output, unsigned int capacity)
{
    int written = 0;
    int remaining = static_cast<int>(capacity) - 1;
    while (remaining != 0)
    {
        const unsigned int lead = static_cast<unsigned char>(*input);
        if (lead == 0)
        {
            output[written] = 0;
            return written + 1;
        }
        if (lead < 0x80)
        {
            output[written++] = static_cast<wchar_t>(static_cast<signed char>(lead));
            --remaining;
            ++input;
        }
        else if (lead - 0xc2 < 0x1e)
        {
            output[written++] = static_cast<wchar_t>(lead * 0x40 +
                static_cast<unsigned char>(input[1]) - 0x3080);
            --remaining;
            input += 2;
        }
        else if (lead - 0xe0 < 0x10)
        {
            output[written++] = static_cast<wchar_t>(lead * 0x1000 +
                static_cast<unsigned char>(input[1]) * 0x40 +
                static_cast<unsigned char>(input[2]) - 0x2080);
            --remaining;
            input += 3;
        }
        else
        {
            output[0] = 0;
            return -1;
        }
    }
    output[written] = 0;
    return written + 1;
}

int __N_19_OnlineCore_Blob_cpp_9428d484::GetNeededUtf8BufferSize(const wchar_t* input)
{
    int size = 1;
    while (*input != 0)
    {
        const unsigned int value = static_cast<unsigned int>(*input++);
        size += value < 0x80 ? 1 : value < 0x800 ? 2 : 3;
    }
    return size;
}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvFromCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const nn::nex::SocketDriver::InetAddress*, const unsigned int*) {}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendToCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const nn::nex::SocketDriver::InetAddress&, const unsigned int*) {}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const unsigned int*) {}
void __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendCallback(nn::nex::SocketDriver::_Result, const unsigned char*, unsigned int, const unsigned int*) {}
