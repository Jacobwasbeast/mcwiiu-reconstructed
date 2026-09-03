#include "vendor/__N_21_FFLiResourceShape_cpp_14c50527.hpp"
#include <string.h>

namespace
{
static uint16_t Swap16(uint16_t value)
{
    return (uint16_t)((value >> 8) | (value << 8));
}

static uint32_t Swap32(uint32_t value)
{
    return ((value >> 24) & 0xffu) | ((value >> 8) & 0xff00u) |
           ((value << 8) & 0xff0000u) | ((value << 24) & 0xff000000u);
}
}

const void* __N_21_FFLiResourceShape_cpp_14c50527::GetElement(unsigned int* elementSize, const void* data, unsigned int offset, unsigned int size)
{
    *elementSize = size;
    if (size == 0)
        return 0;
    return static_cast<const unsigned char*>(data) + offset;
}

void __N_21_FFLiResourceShape_cpp_14c50527::SwapEndianAttribute(void* data, unsigned int format, unsigned int count)
{
    unsigned int componentSize = GetComponentSize((_GX2AttribFormat)format);
    unsigned int components = 1;
    if (format == 0x080d || format == 0x0811 || format == 0x0813)
        components = format == 0x080d ? 2 : 4;
    unsigned char* bytes = static_cast<unsigned char*>(data);
    unsigned int i;
    for (i = 0; i < count * components; ++i)
    {
        if (componentSize == 2)
        {
            uint16_t value;
            memcpy(&value, bytes + i * 2, sizeof(value));
            value = Swap16(value);
            memcpy(bytes + i * 2, &value, sizeof(value));
        }
        else if (componentSize == 4)
        {
            uint32_t value;
            memcpy(&value, bytes + i * 4, sizeof(value));
            value = Swap32(value);
            memcpy(bytes + i * 4, &value, sizeof(value));
        }
    }
}

void __N_21_FFLiResourceShape_cpp_14c50527::SwapEndianIndex(void* data, unsigned int count, unsigned int indexSize)
{
    unsigned int i;
    if (indexSize == 2)
    {
        unsigned char* bytes = static_cast<unsigned char*>(data);
        for (i = 0; i < count; ++i)
        {
            uint16_t value;
            memcpy(&value, bytes + i * 2, sizeof(value));
            value = Swap16(value);
            memcpy(bytes + i * 2, &value, sizeof(value));
        }
    }
    else if (indexSize == 4)
    {
        unsigned char* bytes = static_cast<unsigned char*>(data);
        for (i = 0; i < count; ++i)
        {
            uint32_t value;
            memcpy(&value, bytes + i * 4, sizeof(value));
            value = Swap32(value);
            memcpy(bytes + i * 4, &value, sizeof(value));
        }
    }
}

void __N_21_FFLiResourceShape_cpp_14c50527::SwapEndianVec3(FFLVec3* value)
{
    unsigned char* bytes = static_cast<unsigned char*>(static_cast<void*>(value));
    unsigned int i;
    for (i = 0; i < 3; ++i)
    {
        uint32_t word;
        memcpy(&word, bytes + i * 4, sizeof(word));
        word = Swap32(word);
        memcpy(bytes + i * 4, &word, sizeof(word));
    }
}

void __N_21_FFLiResourceShape_cpp_14c50527::SwapEndianHairTransform(void* value)
{
    if (value != 0)
        SwapEndianAttribute(value, 0x0813, 1);
}

void __N_21_FFLiResourceShape_cpp_14c50527::SwapEndianFacelineTransform(void* value)
{
    if (value != 0)
        SwapEndianAttribute(value, 0x0813, 1);
}

bool __N_21_FFLiResourceShape_cpp_14c50527::IsValid(unsigned int format, unsigned int componentCount, unsigned int stride, unsigned int offset, unsigned int size)
{
    unsigned int componentSize = GetComponentSize((_GX2AttribFormat)format);
    if (componentSize == 0 || componentCount == 0)
        return false;
    if (stride < componentSize * componentCount)
        return false;
    if (offset > size)
        return false;
    return size - offset >= componentSize * componentCount;
}

unsigned int __N_21_FFLiResourceShape_cpp_14c50527::GetComponentSize(_GX2AttribFormat format)
{
    if (format == 0x080d)
        return 4;
    if (format == 0x0811 || format == 0x0813)
        return 4;
    if (format >= 0x020a && format <= 0x020b)
        return 4;
    if (format == 10)
        return 4;
    if (format >= 0x0100 && format < 0x0200)
        return 2;
    if (format >= 0x0000 && format < 0x0100)
        return 1;
    return 0;
}
