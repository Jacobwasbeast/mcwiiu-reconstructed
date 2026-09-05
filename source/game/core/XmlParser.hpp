#ifndef GAME_CORE_XMLPARSER_HPP
#define GAME_CORE_XMLPARSER_HPP

#include <string>

#include "game/core/rapidxml__xml_document_char_.hpp"
#include "game/core/rapidxml__xml_document_wchar_t_.hpp"
#include "game/core/rapidxml__memory_pool_char_.hpp"
#include "game/core/rapidxml__memory_pool_wchar_t_.hpp"

class DataOutputStream;
class File;
class XmlChild;
class XmlObject;

namespace rapidxml
{
    template <class Ch> class xml_node;
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
    rapidxml::memory_pool<char> m_memoryPool;
    rapidxml::xml_document<wchar_t> m_wideDocument;
    rapidxml::memory_pool<wchar_t> m_wideMemoryPool;
};

#endif
