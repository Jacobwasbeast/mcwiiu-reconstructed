#include "game/core/XmlValue.hpp"

bool XmlValue::isString() const
{
    return m_isString;
}

std::wstring XmlValue::asString() const
{
    return m_value;
}
