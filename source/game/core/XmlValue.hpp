#ifndef GAME_CORE_XMLVALUE_HPP
#define GAME_CORE_XMLVALUE_HPP

#include <string>

#include "game/core/XmlConstruct.hpp"

class XmlValue : public XmlConstruct
{
public:
    virtual ~XmlValue();
    virtual bool isString() const;
    virtual std::wstring asString() const;
private:
    friend class XmlObject;
    bool m_isString;
    std::wstring m_value;
};

#endif
