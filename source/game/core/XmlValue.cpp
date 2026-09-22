#include "game/core/XmlValue.hpp"

XmlValue::~XmlValue()
{
    if (m_value.capacity() > 7)
        m_value.shrink_to_fit();
    m_value.clear();
}

bool XmlValue::isString() const
{
    return m_isString;
}

std::wstring XmlValue::asString() const
{
    std::wstring result;
    result.assign(m_value, 0, std::wstring::npos);
    return result;
}
