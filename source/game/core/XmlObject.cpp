#include "game/core/XmlObject.hpp"

std::wstring toLower(const std::wstring& text);

bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second)
{
    return toLower(first) == toLower(second);
}

std::wstring escapeXML(const std::wstring& text)
{
    std::wstring result = text;
    const wchar_t* search[3] = { L"&", L"<", L">" };
    const wchar_t* replacement[3] = { L"&amp;", L"&lt;", L"&gt;" };
    for (int i = 0; i < 3; ++i)
    {
        std::wstring::size_type position = 0;
        while ((position = result.find(search[i], position)) != std::wstring::npos)
        {
            result.replace(position, 1, replacement[i]);
            position += 1;
        }
    }
    return result;
}

std::wstring escape(const std::wstring& text)
{
    std::wstring result = text;
    const wchar_t* search[3] = { L"&", L"<", L">" };
    const wchar_t* replacement[3] = { L"&amp;", L"&lt;", L"&gt;" };
    for (int i = 0; i < 3; ++i)
    {
        std::wstring::size_type position = 0;
        while ((position = result.find(search[i], position)) != std::wstring::npos)
        {
            result.replace(position, 1, replacement[i]);
            position += 1;
        }
    }
    return result;
}

XmlObject::XmlObject(const std::wstring& name)
    : mName(name), mChildren(), mAttrs()
{
}

XmlObject::~XmlObject()
{
    std::vector<XmlChild>::iterator childIterator;
    for (childIterator = mChildren.begin(); childIterator != mChildren.end(); ++childIterator)
    {
        if (childIterator->xml() != NULL)
        {
            delete childIterator->xml();
        }
    }
}

XmlObject* XmlObject::child(const std::wstring& name)
{
    std::vector<XmlChild>::iterator childIterator;
    for (childIterator = mChildren.begin(); childIterator != mChildren.end(); ++childIterator)
    {
        XmlObject* object = childIterator->xml();
        if (object != NULL && (equalsIgnoreCase(object->mName, name) || name.empty()))
            return object;
    }
    return NULL;
}

const XmlObject* XmlObject::child(const std::wstring& name) const
{
    std::vector<XmlChild>::const_iterator childIterator;
    for (childIterator = mChildren.begin(); childIterator != mChildren.end(); ++childIterator)
    {
        const XmlObject* object = childIterator->xml();
        if (object != NULL && (equalsIgnoreCase(object->mName, name) || name.empty()))
            return object;
    }
    return NULL;
}

void XmlObject::addChildRaw(const XmlChild& child)
{
    mChildren.push_back(child);
}

void XmlObject::addAttr(const std::wstring& name, const std::wstring& value)
{
    mAttrs.push_back(std::make_pair(name, value));
}

int XmlObject::size() const
{
    return (int)mChildren.size();
}

XmlValue XmlObject::operator[](const std::wstring& name) const
{
    std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute;
    for (attribute = mAttrs.begin(); attribute != mAttrs.end(); ++attribute)
    {
        if (equalsIgnoreCase(attribute->first, name))
            return XmlValue(attribute->second, true);
    }
    return XmlValue(L"", false);
}

XmlChild XmlObject::operator[](int index) const
{
    if (index < 0 || index >= (int)mChildren.size())
        return XmlChild();
    return mChildren[index];
}

std::wstring XmlObject::asString() const
{
    return (*this)[0].asString();
}

bool XmlObject::isString() const
{
    return size() != 0;
}

std::wstring XmlObject::toString(int indentation, bool formatted) const
{
    std::wstring result;
    result += L"<";
    result += mName;

    std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute;
    for (attribute = mAttrs.begin(); attribute != mAttrs.end(); ++attribute)
    {
        result += L" ";
        result += attribute->first;
        result += L"=\"";
        result += attribute->second;
        result += L"\"";
    }

    if (mChildren.empty())
    {
        result += L"/>\n";
        return result;
    }

    result += L">";
    std::vector<XmlChild>::const_iterator childIterator;
    for (childIterator = mChildren.begin(); childIterator != mChildren.end(); ++childIterator)
    {
        XmlObject* object = childIterator->xml();
        if (object != NULL)
        {
            result += L"\n";
            result += object->toString(indentation + 1, formatted);
        }
        else
        {
            result += escape(childIterator->txt());
        }
    }

    if (mChildren.back().xml() != NULL)
        result += L"\n";
    result += L"</";
    result += mName;
    result += L">";
    return result;
}
