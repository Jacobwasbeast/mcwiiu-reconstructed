#include "game/core/XmlValue.hpp"

XmlValue::~XmlValue()
{
}

std::wstring XmlValue::asString() const
{
    return mValue;
}

bool XmlValue::isString() const
{
    return mIsString;
}
