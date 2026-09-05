#include "platform/wiiu/FFLiDatabaseFileOfficial.hpp"

#include <string.h>

bool FFLiIsSameMiiID(const FFLCreateID* first, const FFLCreateID* second);
bool FFLiIsNullMiiID(const FFLCreateID* createId);
bool FFLiIsTemporaryMiiID(const FFLCreateID* createId);
bool FFLiIsSpecialMiiID(const FFLCreateID* createId);
bool FFLiIsValidMiiID(const FFLCreateID* createId);
bool FFLiIsSameAuthorID(const FFLiAuthorID* first, const FFLiAuthorID* second);
void FFLiPutCRC16(unsigned short seed, void* data, unsigned int size);
bool FFLiIsValidCRC16(const void* data, unsigned int size);
void FFLiMiiDataOfficial2CharInfo(FFLiCharInfo* charInfo, const FFLiMiiDataOfficial& miiData);
bool FFLiVerifyCharInfo(const FFLiCharInfo* charInfo, int mode);
void FFLiCharInfo2MiiDataOfficial(FFLiMiiDataOfficial* miiData, const FFLiCharInfo& charInfo, bool clearAuthor);

namespace
{
void SetNullCreateID(FFLCreateID& createId)
{
    memset(createId.data, 0, sizeof(createId.data));
}

bool IsExistCreateID(const FFLCreateID& createId, const FFLiDatabaseFileOfficial::AdjustRegularBuffer& buffer)
{
    unsigned int i;
    for (i = 0; i < buffer.num; ++i)
        if (FFLiIsSameMiiID(&createId, &buffer.createIds[i]))
            return true;
    return false;
}
}

const FFLiMiiDataOfficial* FFLiDatabaseFileOfficial::GetImpl(unsigned short index) const
{
    return m_miis + index;
}

bool FFLiDatabaseFileOfficial::FindRegularList(unsigned int* index, const FFLCreateID* createId) const
{
    unsigned int i;
    for (i = 0; i < 50; ++i)
    {
        if (FFLiIsSameMiiID(createId, &m_regular[i]))
        {
            if (index != NULL)
                *index = i;
            return true;
        }
    }
    return false;
}

void FFLiDatabaseFileOfficial::UpdateCrc()
{
    FFLiPutCRC16(0, this, 0x43840);
}

void FFLiDatabaseFileOfficial::Init()
{
    m_saveCount = 0;
    m_identifier = 0x46464f43;
    memset(m_extensionData, 0, sizeof(m_extensionData));
    memset(m_miis, 0, sizeof(m_miis));
    memset(m_regular, 0, sizeof(m_regular));
    UpdateCrc();
}

bool FFLiDatabaseFileOfficial::IsValidCrc() const
{
    return FFLiIsValidCRC16(this, 0x43840);
}

bool FFLiDatabaseFileOfficial::IsValidIdentifier() const
{
    return m_identifier == 0x46464f43;
}

bool FFLiDatabaseFileOfficial::IsValid() const
{
    return IsValidIdentifier() && IsValidCrc();
}

bool FFLiDatabaseFileOfficial::Get(FFLiCharInfo* charInfo, unsigned short index, bool includeTemporary, bool includeSpecial) const
{
    if (index < 3000)
    {
        const FFLiMiiDataOfficial* miiData = GetImpl(index);
        if (!FFLiIsNullMiiID(&miiData->createId) &&
            !FFLiIsTemporaryMiiID(&miiData->createId) &&
            (!includeTemporary || includeSpecial || !FFLiIsSpecialMiiID(&miiData->createId)))
        {
            FFLiMiiDataOfficial2CharInfo(charInfo, *miiData);
            if (FFLiVerifyCharInfo(charInfo, 1))
                return true;
        }
    }
    return false;
}

bool FFLiDatabaseFileOfficial::IsAvailable(unsigned short index, bool includeTemporary, bool includeSpecial) const
{
    FFLiCharInfo charInfo;
    return Get(&charInfo, index, includeTemporary, includeSpecial);
}

short FFLiDatabaseFileOfficial::NumOfAvailable(bool includeTemporary, bool includeSpecial) const
{
    short count = 0;
    unsigned short i;
    FFLiCharInfo charInfo;
    for (i = 0; i < 3000; ++i)
    {
        if (Get(&charInfo, i, includeTemporary, includeSpecial))
            ++count;
    }
    return count;
}

bool FFLiDatabaseFileOfficial::Search(unsigned short* index, const FFLCreateID& createId) const
{
    unsigned short i;
    FFLiCharInfo charInfo;
    for (i = 0; i < 3000; ++i)
    {
        if (FFLiIsSameMiiID(&GetImpl(i)->createId, &createId) && Get(&charInfo, i, false, false))
        {
            if (index != NULL)
                *index = i;
            return true;
        }
    }
    return false;
}

bool FFLiDatabaseFileOfficial::IsRegular(unsigned short index, bool includeTemporary, bool includeSpecial) const
{
    FFLiCharInfo charInfo;
    if (Get(&charInfo, index, includeTemporary, includeSpecial))
        return FindRegularList(NULL, (const FFLCreateID*)(charInfo.data + 264));
    return false;
}

bool FFLiDatabaseFileOfficial::AdjustRegularList(AdjustRegularBuffer* buffer)
{
    FFLiCharInfo charInfo;
    unsigned short i;
    unsigned int j;
    bool changed = false;
    buffer->num = 0;
    for (i = 0; i < 3000; ++i)
        if (Get(&charInfo, i, true, true))
            memcpy(buffer->createIds[buffer->num++].data, charInfo.data + 264, sizeof(FFLCreateID));
    for (j = 0; j < 50; ++j)
    {
        if (FFLiIsValidMiiID(&m_regular[j]) && !IsExistCreateID(m_regular[j], *buffer))
        {
            SetNullCreateID(m_regular[j]);
            changed = true;
        }
    }
    return changed;
}
