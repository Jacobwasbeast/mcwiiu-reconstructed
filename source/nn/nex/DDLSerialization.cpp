#include "nn/nex/DDLSerialization.hpp"

#include "nn/nex/qBuffer.hpp"

namespace nn
{
namespace nex
{

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
