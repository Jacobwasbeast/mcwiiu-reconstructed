#ifndef GAME_CORE_XMLVALUE_HPP
#define GAME_CORE_XMLVALUE_HPP

#include <string>

#include "game/core/XmlConstruct.hpp"

class XmlValue : public XmlConstruct
{
public:
    XmlValue(const std::wstring& value, bool isString)
        : mIsString(isString)
        , mValue(value)
    {
    }

    virtual ~XmlValue();

    virtual std::wstring asString() const;
    virtual bool isString() const;

private:
    bool mIsString;
    std::wstring mValue;
};

#endif
