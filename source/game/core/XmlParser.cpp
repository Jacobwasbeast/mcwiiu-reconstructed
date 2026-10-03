#include "game/core/XmlParser.hpp"

#include <cstring>
#include <vector>

#include "game/core/DataOutputStream.hpp"
#include "game/core/XmlChild.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"

unsigned int XmlParser::compare(char* first, char* second, unsigned int length)
{
    unsigned int index;
    for (index = 0; index < length; ++index)
        if (first[index] != second[index])
            return 0;
    return 1;
}

static std::string gUtf8ByteOrderMark("ï»¿");
static std::string gUtf16BigEndianByteOrderMark("þÿ");
static std::string gUtf16LittleEndianByteOrderMark("ÿþ");

unsigned int XmlParser::detectFileFormat(void* data)
{
    char* bytes = (char*)data;
    if (compare(bytes, (char*)gUtf8ByteOrderMark.c_str(), gUtf8ByteOrderMark.size()) == 0)
    {
        if (compare(bytes, (char*)gUtf16BigEndianByteOrderMark.c_str(), gUtf16BigEndianByteOrderMark.size()) != 0)
            return eFileFormatUtf16BigEndian;
        if (compare(bytes, (char*)gUtf16LittleEndianByteOrderMark.c_str(), gUtf16LittleEndianByteOrderMark.size()) != 0)
            return eFileFormatUtf16LittleEndian;
    }
    return eFileFormatUtf8;
}

void XmlParser::_set_file_format(eFileFormat format)
{
    if (format == eFileFormatAuto)
        m_fileFormat = detectFileFormat(m_xmlSrc);
}

void XmlParser::_parse()
{
    if (m_fileFormat == eFileFormatAuto)
        return;
    if (m_fileFormat <= eFileFormatUtf8)
        m_document.parse<0>((char*)m_xmlSrc);
    else if (m_fileFormat <= eFileFormatUtf16LittleEndian)
        m_wideDocument.parse<16>((wchar_t*)(m_xmlSrc + 2));
}

XmlParser::XmlParser(unsigned char* data, unsigned int length)
    : m_sourceName(L"[FROM BYTEARRAY]")
{
    m_xmlSrcLength = length + 2;
    m_xmlSrc = new unsigned char[m_xmlSrcLength];
    std::memset(m_xmlSrc, 0, m_xmlSrcLength);
    std::memcpy(m_xmlSrc, data, length);
    _set_file_format(eFileFormatAuto);
    _parse();
}

XmlParser::~XmlParser()
{
    delete[] m_xmlSrc;
}

std::wstring XmlParser::cnv2Wstr(const char* text)
{
    if (m_fileFormat == eFileFormatAnsi)
        return convStringToWstring(std::string(text));
    if (m_fileFormat == eFileFormatUtf8)
        return convUtf8ToWstring(text);
    return std::wstring(L"__ROOT__");
}

XmlChild XmlParser::_getXml(rapidxml::xml_node<char>* node)
{
    if (node->type() == rapidxml::node_element || node->type() == rapidxml::node_document)
    {
        const char* name = node->name() == NULL ? "" : node->name();
        XmlObject* object = new XmlObject(cnv2Wstr(name));
        rapidxml::xml_attribute<char>* attribute;
        for (attribute = node->first_attribute(); attribute != NULL; attribute = attribute->next_attribute())
        {
            const char* attributeName = attribute->name() == NULL ? "" : attribute->name();
            const char* attributeValue = attribute->value() == NULL ? "" : attribute->value();
            object->addAttr(cnv2Wstr(attributeName), cnv2Wstr(attributeValue));
        }
        rapidxml::xml_node<char>* child;
        for (child = node->first_node(); child != NULL; child = child->next_sibling())
            object->addChildRaw(_getXml(child));
        return XmlChild(object);
    }
    if (node->type() == rapidxml::node_data || node->type() == rapidxml::node_cdata)
        return XmlChild(cnv2Wstr(node->value() == NULL ? "" : node->value()), node->type() == rapidxml::node_data);
    return XmlChild((XmlObject*)NULL);
}

XmlChild XmlParser::_getXml(rapidxml::xml_node<wchar_t>* node)
{
    if (node->type() == rapidxml::node_element || node->type() == rapidxml::node_document)
    {
        const wchar_t* name = node->name() == NULL ? L"" : node->name();
        XmlObject* object = new XmlObject(std::wstring(name));
        rapidxml::xml_attribute<wchar_t>* attribute;
        for (attribute = node->first_attribute(); attribute != NULL; attribute = attribute->next_attribute())
        {
            const wchar_t* attributeName = attribute->name() == NULL ? L"" : attribute->name();
            const wchar_t* attributeValue = attribute->value() == NULL ? L"" : attribute->value();
            object->addAttr(std::wstring(attributeName), std::wstring(attributeValue));
        }
        rapidxml::xml_node<wchar_t>* child;
        for (child = node->first_node(); child != NULL; child = child->next_sibling())
            object->addChildRaw(_getXml(child));
        return XmlChild(object);
    }
    if (node->type() == rapidxml::node_data || node->type() == rapidxml::node_cdata)
        return XmlChild(std::wstring(node->value() == NULL ? L"" : node->value()));
    return XmlChild();
}

XmlObject* XmlParser::getXml()
{
    XmlObject* object = new XmlObject(L"__ROOT__");
    if (m_fileFormat != eFileFormatAuto)
    {
        if (m_fileFormat <= eFileFormatUtf8)
            object->addChildRaw(_getXml(m_document.first_node()));
        else if (m_fileFormat <= eFileFormatUtf16LittleEndian)
            object->addChildRaw(_getXml(m_wideDocument.first_node()));
    }
    return object;
}

XmlObject* XmlParser::readXml(unsigned char* data, unsigned int length)
{
    XmlParser parser(data, length);
    return parser.getXml();
}

void XmlParser::writeXml(DataOutputStream* stream, XmlObject* object, bool formatted)
{
    char* data = (char*)gUtf8ByteOrderMark.c_str();
    unsigned int length = gUtf8ByteOrderMark.size();
    arrayWithLength<unsigned char> output;
    output.data = (unsigned char*)data;
    output.length = length;
    stream->write(output);
    toUtf8(std::wstring(L""), &data, length);
    --length;
    output.data = (unsigned char*)data;
    output.length = length;
    stream->write(output);
    delete[] data;
    const std::vector<XmlChild>& children = object->getChildrenRaw();
    std::vector<XmlChild>::const_iterator child;
    for (child = children.begin(); child != children.end(); ++child)
    {
        std::wstring text = child->xml()->toString(0, true);
        data = NULL;
        length = 0;
        toUtf8(text, &data, length);
        --length;
        output.data = (unsigned char*)data;
        output.length = length;
        stream->write(output);
        delete[] data;
    }
}
