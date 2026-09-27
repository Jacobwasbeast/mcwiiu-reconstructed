#include "game/core/XmlConstruct.hpp"
#include "game/core/StringUtils.hpp"

template<typename T> T _fromString(const std::wstring& text);

XmlConstruct::~XmlConstruct()
{
}

std::wstring XmlConstruct::asString() const
{
    return L"";
}

std::wstring XmlConstruct::asString(const std::wstring& defaultValue) const
{
    if (!isString())
        return defaultValue;

    return asString();
}

int XmlConstruct::asInt() const
{
    return _fromString<int>(asString());
}

int XmlConstruct::asInt(int defaultValue) const
{
    if (isInt())
        return asInt();

    return defaultValue;
}

unsigned int XmlConstruct::asUInt() const
{
    return _fromString<unsigned int>(asString());
}

unsigned int XmlConstruct::asUInt(unsigned int defaultValue) const
{
    if (isUInt())
        return asUInt();

    return defaultValue;
}

bool XmlConstruct::asBool() const
{
    if (!isString())
        return false;

    std::wstring value = asString();
    if (equalsIgnoreCase(value, L"true"))
        return true;

    if (equalsIgnoreCase(value, L"false"))
        return false;

    return false;
}

bool XmlConstruct::asBool(bool defaultValue) const
{
    if (isBool())
        return asBool();

    return defaultValue;
}

double XmlConstruct::asDouble() const
{
    return _fromString<double>(asString());
}

double XmlConstruct::asDouble(double defaultValue) const
{
    if (isDouble())
        return asDouble();

    return defaultValue;
}

float XmlConstruct::asFloat() const
{
    return _fromString<float>(asString());
}

float XmlConstruct::asFloat(float defaultValue) const
{
    if (isFloat())
        return asFloat();

    return defaultValue;
}

bool XmlConstruct::isString() const
{
    return true;
}

bool XmlConstruct::isInt() const
{
    return isString();
}

bool XmlConstruct::isUInt() const
{
    return isString();
}

bool XmlConstruct::isBool() const
{
    return isString();
}

bool XmlConstruct::isDouble() const
{
    return isString();
}

bool XmlConstruct::isFloat() const
{
    return isString();
}

maybe<std::wstring> XmlConstruct::maybeString() const
{
    if (!isString())
        return maybe<std::wstring>();

    return maybe<std::wstring>(asString());
}

maybe<int> XmlConstruct::maybeInt() const
{
    if (!isInt())
        return maybe<int>();

    return maybe<int>(asInt());
}

maybe<unsigned int> XmlConstruct::maybeUInt() const
{
    if (!isUInt())
        return maybe<unsigned int>();

    return maybe<unsigned int>(asUInt());
}

maybe<bool> XmlConstruct::maybeBool() const
{
    if (!isBool())
        return maybe<bool>();

    return maybe<bool>(asBool());
}

maybe<double> XmlConstruct::maybeDouble() const
{
    if (!isDouble())
        return maybe<double>();

    return maybe<double>(asDouble());
}

maybe<float> XmlConstruct::maybeFloat() const
{
    if (!isFloat())
        return maybe<float>();

    return maybe<float>(asFloat());
}
