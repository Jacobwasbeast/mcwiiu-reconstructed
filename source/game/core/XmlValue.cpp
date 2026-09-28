#include "game/core/XmlValue.hpp"

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
