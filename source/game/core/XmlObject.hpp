#ifndef GAME_CORE_XMLOBJECT_HPP
#define GAME_CORE_XMLOBJECT_HPP

#include <string>
#include <utility>
#include <vector>

#include "game/core/WstringLookup.hpp"
#include "game/core/XmlChild.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlValue.hpp"

class XmlObject : public XmlConstruct
{
public:
    XmlObject(const std::wstring& name);
    virtual ~XmlObject();

    XmlObject* child(const std::wstring& name);
    const XmlObject* child(const std::wstring& name) const;
    void addChildRaw(const XmlChild& child);
    void addAttr(const std::wstring& name, const std::wstring& value);
    int size() const;
    XmlValue operator[](const std::wstring& name) const;
    XmlChild operator[](int index) const;
    virtual std::wstring asString() const;
    virtual bool isString() const;
    std::wstring toString(int indentation, bool formatted) const;

    void addChild(XmlObject* child);
    XmlValue getAttr(const std::wstring& name) const;
    std::wstring getText() const;
    void _getLookup(WstringLookup* lookup);
    WstringLookup* getLookup();
    std::vector<XmlChild> getChildren();
    void _getValues(const std::wstring& name, const std::wstring& value, std::vector<std::wstring>* values);
    std::vector<std::wstring> getAllValues(const std::wstring& name);
    std::vector<std::wstring> getValues(const std::wstring& name, const std::wstring& value);
    std::vector<XmlObject*> getChildren(const std::wstring& name);
    const std::vector<XmlChild>& getChildrenRaw() const;
    std::vector<std::pair<std::wstring, std::wstring> >& getAttrs();

    std::wstring mName;

private:
    std::vector<XmlChild> mChildren;
    std::vector<std::pair<std::wstring, std::wstring> > mAttrs;
};

#endif
