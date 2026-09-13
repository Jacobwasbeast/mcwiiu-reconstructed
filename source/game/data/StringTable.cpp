#include "game/data/StringTable.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
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
    if (m_ownsData)
        delete[] m_data;
}

std::wstring StringTable::FindLanguage(bool& found, long long& count)
{
    found = false;
    count = 0;
    std::vector<std::wstring> locale;
    if (g_pApp != NULL)
        g_pApp->getLocale(locale);

    std::wstring selected;
    std::vector<std::pair<std::wstring, int> >::iterator entry;
    for (std::vector<std::wstring>::iterator wanted = locale.begin(); wanted != locale.end(); ++wanted)
    {
        count = 0;
        for (entry = m_entries.begin(); entry != m_entries.end(); ++entry)
        {
            if (entry->first == *wanted)
            {
                found = true;
                return entry->first;
            }
            count += entry->second;
        }
    }

    count = 0;
    return selected;
}

std::wstring StringTable::ReadLanguageFromStream(DataInputStream* stream)
{
    int format = stream->readInt();
    m_isLanguageTable = false;
    if (format > 0)
        m_isLanguageTable = stream->readByte() != 0;
    std::wstring language = stream->readUTF();
    _LanguageData& data = m_languages[language];
    int count = stream->readInt();
    data.stringsById.clear();
    data.strings.clear();
    data.orderedStrings.clear();
    data.count = count;
    for (int i = 0; i < count; ++i)
    {
        std::wstring key;
        if (format > 0 && m_isLanguageTable && i < (int)m_ids.size())
            key = L"";
        else if (format > 0 && i < (int)m_keys.size())
            key = m_keys[i];
        else
            key = stream->readUTF();
        std::wstring value = stream->readUTF();
        if (format > 0 && m_isLanguageTable && i < (int)m_ids.size())
        {
            data.stringsById[m_ids[i]] = value;
        }
        else
        {
            data.strings[key] = value;
            data.orderedStrings.push_back(key);
        }
    }
    return language;
}

void StringTable::SelectLanguage(const std::wstring& language)
{
    boost::unordered_map<std::wstring, _LanguageData>::iterator it = m_languages.find(language);
    if (it == m_languages.end())
    {
        m_strings = NULL;
        return;
    }
    m_strings = &it->second.strings;
    m_stringsById = &it->second.stringsById;
}

void StringTable::ReadAllFromStream(DataInputStream* stream)
{
    int version = stream->readInt();
    int languageCount = stream->readInt();
    m_isLanguageTable = false;
    m_ids.clear();
    m_keys.clear();
    m_entries.clear();
    if (version == 2)
    {
        m_isLanguageTable = stream->readByte() != 0;
        int count = stream->readInt();
        for (int i = 0; i < count; ++i)
        {
            if (m_isLanguageTable)
                m_ids.push_back(stream->readInt());
            else
                m_keys.push_back(stream->readUTF());
        }
    }
    for (int i = 0; i < languageCount; ++i)
    {
        std::wstring language = stream->readUTF();
        int size = stream->readInt();
        m_entries.push_back(std::make_pair(language, size));
    }
    bool found = false;
    long long offset = 0;
    std::wstring language = FindLanguage(found, offset);
    if (m_localized)
    {
        for (int i = 0; i < languageCount; ++i)
            ReadLanguageFromStream(stream);
    }
    else if (found)
    {
        stream->skip(offset);
        ReadLanguageFromStream(stream);
    }
    if (found)
        SelectLanguage(language);
}

void StringTable::ProcessStringTableData()
{
    if (m_data == NULL || m_dataLength == 0)
    {
        bool found = false;
        long long ignored = 0;
        std::wstring language = FindLanguage(found, ignored);
        if (found)
            SelectLanguage(language);
        return;
    }
    m_strings = NULL;
    m_languages.clear();
    m_ids.clear();
    m_keys.clear();
    m_entries.clear();
    m_stringsById = NULL;

    unsigned char* copy = new unsigned char[m_dataLength];
    std::memcpy(copy, m_data, m_dataLength);
    arrayWithLength<unsigned char> input(copy, m_dataLength);
    ByteArrayInputStream byteStream(input);
    DataInputStream stream(&byteStream);
    ReadAllFromStream(&stream);
    stream.close();
}

void StringTable::ReloadStringTable()
{
    if (!m_localized)
    {
        if (m_strings != NULL)
            m_strings->clear();
        if (m_stringsById != NULL)
            m_stringsById->clear();
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
    m_strings = NULL;
    m_stringsById = NULL;
    m_languages.clear();
    m_ids.clear();
    m_keys.clear();
    if (m_strings != NULL) m_strings->clear();
    if (m_stringsById != NULL) m_stringsById->clear();
}
