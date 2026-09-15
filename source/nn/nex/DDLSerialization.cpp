#include "nn/nex/DDLSerialization.hpp"

#include "nn/nex/qBuffer.hpp"
#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/StringConverter.hpp"
#include "nn/nex/SystemSetting.hpp"

namespace nn
{
namespace nex
{

SystemSetting g_ddlConnectionEncoding(L"ConnectionEncoding", 1);
unsigned int g_ddlStringEncoding = 2;

namespace
{

bool ShouldConvertForUTF8Connection()
{
    return g_ddlConnectionEncoding.m_value == 1;
}

bool ShouldConvertForLatin1Connection()
{
    return g_ddlConnectionEncoding.m_value == 0 && g_ddlStringEncoding == 1;
}

}

void _Type_string::Add(Message* pMessage, const String& rSource)
{
    const wchar_t* pSource = rSource;
    if (pSource == 0)
    {
        const std::uint16_t uiSize = 0;
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), 2, 1);
        return;
    }

    if (ShouldConvertForUTF8Connection())
    {
        StringConverter converter(0);
        converter.Encode(pSource, false);
        const std::uint16_t uiSize = static_cast<std::uint16_t>(converter.GetBufferSize());
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), 2, 1);
        pMessage->Append(reinterpret_cast<const unsigned char*>(converter.GetBuffer()),
                         1, converter.GetBufferSize());
        return;
    }

    const std::uint16_t uiSize = static_cast<std::uint16_t>(rSource.GetLength() + 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), 2, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(pSource), 2, uiSize);
}

bool _Type_string::Extract(Message* pMessage, String* pDestination)
{
    std::uint16_t uiSize;
    if (pMessage->ExtractPrimitive(&uiSize))
    {
        if (!pMessage->ValidateBufferLimit(uiSize))
            return false;

        if (uiSize != 0)
        {
            pDestination->Reserve(uiSize + 1);
            if (ShouldConvertForUTF8Connection() && !ShouldConvertForLatin1Connection())
            {
                StringConverter converter(uiSize + 1);
                pMessage->Extract(reinterpret_cast<unsigned char*>(converter.GetBuffer()),
                                  1, uiSize);
                converter.Decode(*pDestination, uiSize, false);
            }
            else if (ShouldConvertForLatin1Connection())
            {
                StringConverter converter(0);
                wchar_t* pBuffer = SpecialNewArray<wchar_t>(uiSize, L"", 0);
                pMessage->Extract(reinterpret_cast<unsigned char*>(pBuffer), 2, uiSize);
                converter.Encode(pBuffer, false);
                *pDestination = converter.GetBuffer();
                SpecialDeleteArray(pBuffer);
            }
            else
            {
                wchar_t* pBuffer = *pDestination;
                pMessage->Extract(reinterpret_cast<unsigned char*>(pBuffer), 2, uiSize);
            }
        }
    }
    return true;
}

void _Type_qBuffer::Add(Message* pMessage, const qBuffer& rSource)
{
    std::uint16_t uiSize = 0;

    if (!rSource.empty())
    {
        uiSize = static_cast<std::uint16_t>(rSource.size());
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), sizeof(uiSize), 1);

    if (!rSource.empty())
    {
        pMessage->Append(rSource.front(), 1, rSource.size());
    }
}

void _Type_qBuffer::Extract(Message* pMessage, qBuffer* pDestination)
{
    std::uint16_t uiSize = 0;

    pMessage->ExtractPrimitive(&uiSize);

    if (!pMessage->ValidateBufferLimit(uiSize))
    {
        return;
    }

    pDestination->clear();

    if (uiSize != 0)
    {
        const Buffer* pBuffer = pMessage->GetBuffer();

        pDestination->push_back(pBuffer->GetData() + pMessage->GetPosition(), uiSize);

        const unsigned int uiPosition = pMessage->GetPosition() + uiSize;

        if (uiPosition <= pMessage->GetLength())
        {
            pMessage->SetPosition(uiPosition);
        }
    }
}

}
}
