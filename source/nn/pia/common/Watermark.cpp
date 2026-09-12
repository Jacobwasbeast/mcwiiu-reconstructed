#include "nn/pia/Reconstruction.hpp"

#include <string.h>

namespace
{

long long maximum(long long lhs, long long rhs)
{
    return lhs < rhs ? rhs : lhs;
}

long long minimum(long long lhs, long long rhs)
{
    return rhs < lhs ? rhs : lhs;
}

}

namespace nn { namespace pia { namespace common {

Watermark::Watermark()
{
    m_enabled = false;
    SetName("(No name)");
    Reset();
}

void Watermark::SetName(const char* name)
{
    unsigned int length = static_cast<unsigned int>(strlen(name));
    if (length > cNameLengthMax) length = cNameLengthMax;
    memcpy(m_name, name, length);
    m_name[length] = '\0';
}

void Watermark::Reset()
{
    m_minimum = 0x7fffffffffffffffLL;
    m_maximum = -0x7fffffffffffffffLL - 1;
    m_current = 0;
    m_updateCount = 0;
}

void Watermark::Update(long long value)
{
    if (!m_enabled) return;

    m_maximum = maximum(m_maximum, value);
    m_minimum = minimum(m_minimum, value);
    m_current = value;
    ++m_updateCount;
}

Watermark* WatermarkManager::GetWatermark(int index)
{
    if (static_cast<unsigned int>(index) >= cWatermarkNum) return 0;
    return &m_watermarks[index];
}

}}}
