#include "game/data/StringTable.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/Random.hpp"

#include <cstring>

StringTable::StringTable() : m_isLanguageTable(false), m_localized(false), m_ownsData(false), m_strings(NULL), m_stringsById(NULL), m_data(NULL), m_dataLength(0)
{
}

StringTable::StringTable(arrayWithLength<unsigned char> data, bool localized, bool borrowed) : m_isLanguageTable(false), m_localized(localized), m_ownsData(false), m_strings(NULL), m_stringsById(NULL), m_data(NULL), m_dataLength(0)
{
    if (!borrowed)
    {
        m_ownsData = true;
        m_dataLength = data.length;
        m_data = new unsigned char[m_dataLength]();
        if (m_dataLength != 0 && data.data != NULL)
            std::memcpy(m_data, data.data, m_dataLength);
    }
    else
    {
        m_data = data.data;
        m_dataLength = data.length;
    }
    ProcessStringTableData();
    if (m_ownsData)
    {
        delete[] m_data;
        m_data = NULL;
        m_dataLength = 0;
    }
}

StringTable::~StringTable()
{
    delete m_strings;
    delete m_stringsById;
    if (m_ownsData)
        delete[] m_data;
}

std::wstring StringTable::FindLanguage(bool& found, long long& count)
{
    found = false;
    count = 0;
    std::wstring selected;
    if (m_languages.empty())
        return selected;
    boost::unordered_map<std::wstring, _LanguageData>::iterator it = m_languages.begin();
    selected = it->first;
    found = true;
    return selected;
}

std::wstring StringTable::ReadLanguageFromStream(DataInputStream* stream)
{
    int version = stream->readInt();
    m_isLanguageTable = stream->readByte() != 0;
    std::wstring language = stream->readUTF();
    _LanguageData& data = m_languages[language];
    int count = stream->readInt();
    int i;
    for (i = 0; i < count; ++i)
    {
        std::wstring key = stream->readUTF();
        std::wstring value = version < 2 ? stream->readUTF() : stream->readUTF();
        data.strings[key] = value;
        data.orderedStrings.push_back(key);
    }
    data.count = count;
    return language;
}

void StringTable::SelectLanguage(const std::wstring& language)
{
    _LanguageData& data = m_languages[language];
    m_strings = &data.strings;
    if (m_stringsById == NULL)
        m_stringsById = new boost::unordered_map<int, std::wstring>();
}

void StringTable::ReadAllFromStream(DataInputStream* stream)
{
    int count = stream->readInt();
    int i;
    for (i = 0; i < count; ++i)
        ReadLanguageFromStream(stream);
    bool found = false;
    long long bytes = 0;
    std::wstring language = FindLanguage(found, bytes);
    if (found)
        SelectLanguage(language);
}

void StringTable::ProcessStringTableData()
{
    if (m_data == NULL || m_dataLength == 0)
        return;
    arrayWithLength<unsigned char> input;
    input.data = m_data;
    input.length = m_dataLength;
    m_languages.clear();
    m_ids.clear();
    m_keys.clear();
    DataInputStream* stream = NULL;
    if (stream != NULL)
        ReadAllFromStream(stream);
}

void StringTable::ReloadStringTable()
{
    if (!m_localized)
    {
        if (m_strings != NULL) m_strings->clear();
        if (m_stringsById != NULL) m_stringsById->clear();
    }
    ProcessStringTableData();
}

void StringTable::getData(unsigned char** data, unsigned int* length)
{
    *data = m_data;
    *length = m_dataLength;
}

wchar_t* StringTable::getString(const std::wstring& key)
{
    if (m_strings != NULL)
    {
        boost::unordered_map<std::wstring, std::wstring>::iterator it = m_strings->find(key);
        if (it != m_strings->end())
            return const_cast<wchar_t*>(it->second.c_str());
    }
    return const_cast<wchar_t*>(L"");
}

const wchar_t* StringTable::getString(int id)
{
    if (id == -1 || id == -2 || m_stringsById == NULL)
        return L"";
    boost::unordered_map<int, std::wstring>::iterator it = m_stringsById->find(id);
    return it == m_stringsById->end() ? L"" : it->second.c_str();
}

unsigned int StringTable::getRandomStringId(Random* random)
{
    if (m_stringsById == NULL || m_stringsById->empty())
        return 0;
    unsigned int selected = (unsigned int)random->nextInt((int)m_stringsById->size());
    boost::unordered_map<int, std::wstring>::iterator it = m_stringsById->begin();
    while (selected-- != 0) ++it;
    return (unsigned int)it->first;
}

std::vector<int> StringTable::getRandomStringIds(int count, Random* random)
{
    std::vector<int> result;
    while ((int)result.size() < count && (m_stringsById == NULL || result.size() < m_stringsById->size()))
    {
        int id = (int)getRandomStringId(random);
        bool duplicate = false;
        std::vector<int>::iterator it;
        for (it = result.begin(); it != result.end(); ++it) if (*it == id) duplicate = true;
        if (!duplicate) result.push_back(id);
    }
    return result;
}

unsigned long long StringTable::copyStringToTable(const std::wstring& key, StringTable* table)
{
    if (table == NULL || m_strings == NULL) return 0;
    std::wstring value;
    boost::unordered_map<std::wstring, std::wstring>::iterator it = m_strings->find(key);
    if (it != m_strings->end()) value = it->second;
    if (table->m_strings == NULL) table->m_strings = new boost::unordered_map<std::wstring, std::wstring>();
    (*table->m_strings)[key] = value;
    return (unsigned long long)(unsigned long)this;
}

void StringTable::write(DataOutputStream* stream)
{
    stream->writeInt(2);
    stream->writeBoolean(m_isLanguageTable);
    stream->writeInt((int)m_languages.size());
    boost::unordered_map<std::wstring, _LanguageData>::iterator lang;
    for (lang = m_languages.begin(); lang != m_languages.end(); ++lang)
    {
        stream->writeUTF(lang->first);
        stream->writeInt((int)lang->second.strings.size());
        boost::unordered_map<std::wstring, std::wstring>::iterator item;
        for (item = lang->second.strings.begin(); item != lang->second.strings.end(); ++item)
        {
            stream->writeUTF(item->first);
            stream->writeUTF(item->second);
        }
    }
}

void StringTable::Reset()
{
    m_languages.clear();
    m_ids.clear();
    m_keys.clear();
    if (m_strings != NULL) m_strings->clear();
    if (m_stringsById != NULL) m_stringsById->clear();
}
