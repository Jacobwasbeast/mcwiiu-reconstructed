#include "nn/pia/common/Hmac.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

HashContextBase::~HashContextBase()
{
}

Hmac::Hmac()
    : m_context(0)
{
    Initialize(0, 0, 0);
}

void Hmac::Initialize(HashContextBase* context, const void* key, unsigned int keySize)
{
    m_context = context;
    memset(m_innerPad, 0x36, sizeof(m_innerPad));
    memset(m_outerPad, 0x5c, sizeof(m_outerPad));
    if (context == 0 || key == 0 || keySize == 0)
        return;

    unsigned char block[64];
    memset(block, 0, sizeof(block));
    if (keySize > sizeof(block))
    {
        context->Initialize();
        context->Update(key, keySize);
        context->GetHash(block);
        keySize = context->GetHashSize();
    }
    else
    {
        memcpy(block, key, keySize);
    }
    for (unsigned int index = 0; index < sizeof(block); ++index)
    {
        m_innerPad[index] = static_cast<unsigned char>(block[index] ^ 0x36);
        m_outerPad[index] = static_cast<unsigned char>(block[index] ^ 0x5c);
    }
}

void Hmac::Calc(void* hash, const void* data, unsigned int size)
{
    if (m_context == 0)
        return;

    unsigned char inner[64];
    m_context->Initialize();
    m_context->Update(m_innerPad, sizeof(m_innerPad));
    if (data != 0 && size != 0)
        m_context->Update(data, size);
    m_context->GetHash(inner);

    m_context->Initialize();
    m_context->Update(m_outerPad, sizeof(m_outerPad));
    m_context->Update(inner, m_context->GetHashSize());
    m_context->GetHash(hash);
}

}}}
