#include "game/data/StringTable.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/StringConversion.hpp"
#include "game/util/Random.hpp"

#include <algorithm>
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
    std::vector<std::wstring> localeNames;
    if (g_pApp != NULL)
        g_pApp->getLocale(localeNames);

    std::wstring selected;
    std::vector<std::pair<std::wstring, int> >::iterator entry;
    for (std::vector<std::wstring>::iterator localeName = localeNames.begin();
         localeName != localeNames.end(); ++localeName)
    {
        count = 0;
        for (entry = m_entries.begin(); entry != m_entries.end(); ++entry)
        {
            if (entry->first == *localeName)
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
    int count = stream->readInt();
    _LanguageData& data = m_languages[language];
    if (!m_isLanguageTable)
    {
        if (format < 2)
        {
            for (int i = 0; i < count; ++i)
            {
                std::wstring key = stream->readUTF();
                std::wstring value = stream->readUTF();
                data.strings[key] = value;
            }
        }
        else
        {
            for (int i = 0; i < count; ++i)
            {
                std::wstring key = m_keys[i];
                std::wstring value = stream->readUTF();
                data.strings[key] = value;
            }
        }
    }
    else if (format < 1)
    {
        for (int i = 0; i < count; ++i)
        {
            std::wstring value = stream->readUTF();
            data.orderedStrings.push_back(value);
        }
    }
    else if (!m_ids.empty())
    {
        for (int i = 0; i < count; ++i)
        {
            int id = m_ids[i];
            std::wstring value = stream->readUTF();
            data.stringsById[id] = value;
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
        if (m_languages.empty())
        {
            for (int i = 0; i < languageCount; ++i)
                ReadLanguageFromStream(stream);
        }
    }
    else if (found)
    {
        stream->skip(offset);
        ReadLanguageFromStream(stream);
    }
    if (found)
        SelectLanguage(language);
    m_keys.clear();
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
    stream->writeInt(static_cast<int>(m_languages.size()));
    if ((m_strings == NULL || m_stringsById == NULL) && !m_languages.empty())
    {
        _LanguageData& first = m_languages.begin()->second;
        m_strings = &first.strings;
        m_stringsById = &first.stringsById;
    }

    stream->writeBoolean(m_isLanguageTable);
    if (m_isLanguageTable)
    {
        stream->writeInt(static_cast<int>(m_stringsById->size()));
        for (boost::unordered_map<int, std::wstring>::const_iterator item = m_stringsById->begin();
             item != m_stringsById->end(); ++item)
        {
            stream->writeInt(item->first);
        }
    }
    else
    {
        stream->writeInt(static_cast<int>(m_strings->size()));
        for (boost::unordered_map<std::wstring, std::wstring>::const_iterator item = m_strings->begin();
             item != m_strings->end(); ++item)
        {
            stream->writeUTF(item->first);
        }
    }

    boost::unordered_map<std::wstring, ByteArrayOutputStream*> languageStreams;
    for (boost::unordered_map<std::wstring, _LanguageData>::iterator lang = m_languages.begin();
         lang != m_languages.end(); ++lang)
    {
        ByteArrayOutputStream* bytes = new ByteArrayOutputStream();
        DataOutputStream languageStream(bytes);
        languageStream.writeInt(2);
        languageStream.writeBoolean(m_isLanguageTable);
        languageStream.writeUTF(lang->first);
        const _LanguageData& data = lang->second;
        if (m_isLanguageTable)
        {
            languageStream.writeInt(static_cast<int>(data.stringsById.size()));
            for (boost::unordered_map<int, std::wstring>::const_iterator item = data.stringsById.begin();
                 item != data.stringsById.end(); ++item)
            {
                languageStream.writeUTF(item->second);
            }
        }
        else
        {
            languageStream.writeInt(static_cast<int>(data.strings.size()));
            for (boost::unordered_map<std::wstring, std::wstring>::const_iterator item = data.strings.begin();
                 item != data.strings.end(); ++item)
            {
                languageStream.writeUTF(item->second);
            }
        }
        languageStreams.insert(std::make_pair(lang->first, bytes));
    }

    std::vector<std::wstring> languageNames;
    for (boost::unordered_map<std::wstring, _LanguageData>::iterator lang = m_languages.begin();
         lang != m_languages.end(); ++lang)
    {
        languageNames.push_back(lang->first);
    }
    std::sort(languageNames.begin(), languageNames.end(), compareStrings);

    for (std::vector<std::wstring>::iterator name = languageNames.begin(); name != languageNames.end(); ++name)
    {
        std::wstring language = *name;
        stream->writeUTF(language);
        stream->writeInt(languageStreams[language]->mCount);
    }
    for (std::vector<std::wstring>::iterator name = languageNames.begin(); name != languageNames.end(); ++name)
    {
        std::wstring language = *name;
        ByteArrayOutputStream* bytes = languageStreams[language];
        stream->write(arrayWithLength<unsigned char>(bytes->mBuffer.data, bytes->mCount));
        delete bytes;
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
