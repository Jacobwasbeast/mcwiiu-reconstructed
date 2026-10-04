#include "nn/nex/KeyedChecksumAlgorithm.hpp"

namespace nn
{
namespace nex
{

KeyedChecksumAlgorithm::KeyedChecksumAlgorithm(unsigned int uiMinKeyLength,
    unsigned int uiMaxKeyLength)
    : m_uiMinKeyLength(uiMinKeyLength)
    , m_uiMaxKeyLength(uiMaxKeyLength)
{
}

KeyedChecksumAlgorithm::~KeyedChecksumAlgorithm()
{
}

bool KeyedChecksumAlgorithm::IsReady() const
{
    unsigned int uiLength = m_key.GetLength();
    return m_uiMinKeyLength <= uiLength && uiLength <= m_uiMaxKeyLength;
}

bool KeyedChecksumAlgorithm::SetKey(const Key& rKey)
{
    unsigned int uiLength = rKey.GetLength();
    if (m_uiMinKeyLength <= uiLength && uiLength <= m_uiMaxKeyLength)
    {
        m_key = rKey;
        KeyHasChanged();
        return true;
    }

    return false;
}

}
}
