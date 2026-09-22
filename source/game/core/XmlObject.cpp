#include "game/core/XmlObject.hpp"

#include "game/core/XmlChild.hpp"
#include "game/core/XmlValue.hpp"

#include <stdexcept>

XmlObject gXmlObjectNull(L"");

bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second);

namespace
{
std::wstring escapeXml(const std::wstring& text)
{
    std::wstring escaped;
    for (std::wstring::const_iterator character = text.begin(); character != text.end(); ++character)
    {
        switch (*character)
        {
        case L'&': escaped += L"&amp;"; break;
        case L'<': escaped += L"&lt;"; break;
        case L'>': escaped += L"&gt;"; break;
        case L'"': escaped += L"&quot;"; break;
        case L'\'': escaped += L"&apos;"; break;
        default: escaped += *character; break;
        }
    }
    return escaped;
}

void appendIndent(std::wstring& output, int indent)
{
    for (int level = 0; level < indent; ++level)
        output += L"\t";
}
}

XmlObject::XmlObject(const std::wstring& name)
    : mName(name)
{
}

XmlObject::~XmlObject()
{
    for (std::vector<XmlChild>::iterator item = m_children.begin(); item != m_children.end(); ++item)
        if (item->m_type == 1 && item->xml() != 0)
            delete item->xml();
}

XmlObject* XmlObject::child(const std::wstring& name)
{
    for (std::vector<XmlChild>::iterator item = m_children.begin(); item != m_children.end(); ++item)
        if (item->m_type == 1 && item->xml() != 0)
        {
            if (equalsIgnoreCase(item->xml()->mName, name) || name.empty())
                return item->xml();
        }
    return &gXmlObjectNull;
}

const XmlObject* XmlObject::child(const std::wstring& name) const
{
    for (std::vector<XmlChild>::const_iterator item = m_children.begin(); item != m_children.end(); ++item)
        if (item->m_type == 1 && item->xml() != 0)
        {
            if (equalsIgnoreCase(item->xml()->mName, name) || name.empty())
                return item->xml();
        }
    return &gXmlObjectNull;
}

void XmlObject::addChildRaw(const XmlChild& child)
{
    m_children.push_back(child);
}

void XmlObject::addAttr(const std::wstring& name, const std::wstring& value)
{
    m_attrs.push_back(std::make_pair(name, value));
}

int XmlObject::size() const
{
    return static_cast<int>(m_children.size());
}

const XmlChild& XmlObject::operator[](int index) const
{
    return m_children.at(static_cast<std::size_t>(index));
}

XmlValue XmlObject::operator[](const std::wstring& name) const
{
    XmlValue result;
    result.m_isString = 0;
    for (std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute = m_attrs.begin(); attribute != m_attrs.end(); ++attribute)
        if (equalsIgnoreCase(attribute->first, name))
        {
            result.m_isString = 1;
            result.m_value = attribute->second;
            break;
        }
    return result;
}

std::wstring XmlObject::asString() const
{
    return (*this)[0].asString();
}

bool XmlObject::isString() const
{
    return size() != 0;
}

std::wstring XmlObject::toString(int indent, bool formatted) const
{
    std::wstring output;
    if (indent >= 0)
    {
        output.reserve(static_cast<std::size_t>(indent));
        appendIndent(output, indent);
    }
    output += L"<";
    output += mName;
    for (std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute = m_attrs.begin(); attribute != m_attrs.end(); ++attribute)
    {
        output += L" ";
        output += attribute->first;
        output += L"=\"";
        output += attribute->second;
        output += L"\"";
    }
    if (m_children.empty())
        return output + L"/>\n";
    output += L">";
    for (std::vector<XmlChild>::const_iterator item = m_children.begin(); item != m_children.end(); ++item)
    {
        if (item->m_type == 0)
        {
            const std::wstring text = item->txt();
            const std::wstring escaped = escapeXml(text);
            output += escaped;
        }
        else if (item->m_type == 2)
        {
            if (!formatted)
                output += L"<![CDATA[";
            const std::wstring text = item->txt();
            const std::wstring escaped = escapeXml(text);
            output += escaped;
            if (!formatted)
                output += L"]]>";
        }
        else
        {
            output += L"\n";
            output += item->xml()->toString(indent + 1, false);
            output += L"\n";
        }
    }
    output += L"</";
    output += mName;
    output += L">";
    if (m_children.back().m_type == 1)
        appendIndent(output, indent);
    return output;
}

const std::vector<XmlChild>& XmlObject::getChildrenRaw() const
{
    return m_children;
}
