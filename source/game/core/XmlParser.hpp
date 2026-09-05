#ifndef GAME_CORE_XMLPARSER_HPP
#define GAME_CORE_XMLPARSER_HPP

#include <string>

class DataOutputStream;
class File;
class XmlChild;
class XmlObject;

namespace rapidxml
{
    enum node_type
    {
        node_document,
        node_element,
        node_data
    };

    template <class Ch> class xml_attribute;

    template <class Ch> class xml_node
    {
    public:
        node_type type() const;
        const Ch* name() const;
        const Ch* value() const;
        xml_node<Ch>* first_node(const Ch* name = 0, unsigned int name_size = 0, bool case_sensitive = true) const;
        xml_node<Ch>* next_sibling(const Ch* name = 0, unsigned int name_size = 0, bool case_sensitive = true) const;
        xml_attribute<Ch>* first_attribute(const Ch* name = 0, unsigned int name_size = 0, bool case_sensitive = true) const;
        void remove_all_nodes();
        void remove_all_attributes();
    };

    template <class Ch> class memory_pool
    {
    public:
        void clear();
    };

    template <class Ch> class xml_document : public xml_node<Ch>, public memory_pool<Ch>
    {
    public:
        template <int Flags> void parse(Ch* text);
        void clear();
    };

    template <class Ch> class xml_attribute : public xml_node<Ch>
    {
    public:
        xml_attribute<Ch>* next_attribute(const Ch* name = 0, unsigned int name_size = 0, bool case_sensitive = true) const;
    };
}

class XmlParser
{
public:
    enum eFileFormat
    {
        eFileFormatAuto = 0,
        eFileFormatAnsi = 1,
        eFileFormatUtf8 = 2,
        eFileFormatUtf16LittleEndian = 3,
        eFileFormatUtf16BigEndian = 4
    };

    XmlParser(unsigned char* data, unsigned int length);
    ~XmlParser();
    static unsigned int compare(char* first, char* second, unsigned int length);
    unsigned int detectFileFormat(void* data);
    void _set_file_format(eFileFormat format);
    void _parse();
    std::wstring cnv2Wstr(const char* text);
    XmlChild _getXml(rapidxml::xml_node<char>* node);
    XmlChild _getXml(rapidxml::xml_node<wchar_t>* node);
    XmlObject* getXml();
    static XmlObject* readXml(unsigned char* data, unsigned int length);
    static void writeXml(DataOutputStream* stream, XmlObject* object, bool formatted);
    void _init();
    void _readFile(File* file);
    XmlParser(File* file);
    XmlParser(File* file, eFileFormat format);
    unsigned char* getXmlSrc();
    static XmlObject* readXml(File* file);
    static XmlObject* readXml(std::wstring file);
    static bool is_big_endian();
    static void writeXml(File* file, XmlObject* object, bool formatted);
    static void writeXml(std::wstring file, XmlObject* object, bool formatted);
    static void writeXml(unsigned char*& data, unsigned int& length, XmlObject* object, bool formatted);

private:
    std::wstring m_sourceName;
    eFileFormat m_fileFormat;
    unsigned char* m_xmlSrc;
    unsigned int m_xmlSrcLength;
    rapidxml::xml_document<char> m_document;
    rapidxml::xml_document<wchar_t> m_wideDocument;
};

#endif
