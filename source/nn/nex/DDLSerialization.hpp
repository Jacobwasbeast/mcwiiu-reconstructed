#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DDLSERIALIZATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DDLSERIALIZATION_HPP

#include <cstdint>

#include "nn/nex/Message.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class String;
class qBuffer;
class SystemSetting;

extern SystemSetting g_ddlConnectionEncoding;
extern unsigned int g_ddlStringEncoding;

class _Type_byte
{
public:
    typedef unsigned char CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination)
    {
        pMessage->ExtractPrimitive(pDestination);
    }
};

class _Type_uint16
{
public:
    typedef std::uint16_t CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination)
    {
        pMessage->ExtractPrimitive(pDestination);
    }
};

class _Type_uint32
{
public:
    typedef std::uint32_t CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination)
    {
        pMessage->ExtractPrimitive(pDestination);
    }
};

class _Type_uint64
{
public:
    typedef std::uint64_t CPPType;

    static void Add(Message* pMessage, const CPPType& rSource)
    {
        pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource), sizeof(CPPType), 1);
    }

    static void Extract(Message* pMessage, CPPType* pDestination)
    {
        pMessage->ExtractPrimitive(pDestination);
    }
};

class _Type_string
{
public:
    typedef String CPPType;

    static void Add(Message* pMessage, const String& rSource);
    static bool Extract(Message* pMessage, String* pDestination);
};

class _Type_qBuffer
{
public:
    typedef qBuffer CPPType;

    static void Add(Message* pMessage, const qBuffer& rSource);
    static void Extract(Message* pMessage, qBuffer* pDestination);
};

template <typename Type>
class _Type_qvector
{
public:
    typedef typename Type::CPPType CPPType;

    static void Add(Message* pMessage, const qVector<CPPType>& rSource)
    {
        const std::uint32_t uiCount = static_cast<std::uint32_t>(rSource.size());

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCount), sizeof(uiCount), 1);

        for (typename qVector<CPPType>::const_iterator it = rSource.begin();
            it != rSource.end(); ++it)
        {
            Type::Add(pMessage, *it);
        }
    }

    static void Extract(Message* pMessage, qVector<CPPType>* pDestination)
    {
        pDestination->clear();

        std::uint32_t uiCount = 0;

        if (!pMessage->ExtractPrimitive(&uiCount))
        {
            return;
        }

        pDestination->reserve(uiCount);

        for (std::uint32_t uiIndex = 0; uiIndex < uiCount && !pMessage->HasError(); ++uiIndex)
        {
            CPPType element;

            Type::Extract(pMessage, &element);

            if (pMessage->HasError())
            {
                pDestination->clear();
                return;
            }

            pDestination->push_back(element);
        }
    }
};

}
}

#endif
