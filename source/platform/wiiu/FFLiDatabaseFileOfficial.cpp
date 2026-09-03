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
void SetNullCreateID(FFLCreateID& createId);

FFLiMiiDataOfficial* FFLiDatabaseFileOfficial::GetImpl(unsigned short index)
{
    return m_miis + index;
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

void FFLiDatabaseFileOfficial::DeleteRegularList(const FFLCreateID* createId)
{
    unsigned int index;
    if (FindRegularList(&index, createId))
    {
        SetNullCreateID(m_regular[index]);
        UpdateSaveCount();
    }
}

bool FFLiDatabaseFileOfficial::FindEmptyRegularList(unsigned int* index) const
{
    unsigned int i;
    for (i = 0; i < 50; ++i)
    {
        if (!FFLiIsValidMiiID(&m_regular[i]))
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

void FFLiDatabaseFileOfficial::UpdateSaveCount()
{
    ++m_saveCount;
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

void FFLiDatabaseFileOfficial::DeleteMii(FFLiMiiDataOfficial* miiData, unsigned short index)
{
    if (index < 3000)
    {
        if (miiData != NULL)
            *miiData = *GetImpl(index);
        memset(GetImpl(index), 0, sizeof(FFLiMiiDataOfficial));
        UpdateSaveCount();
    }
}

bool FFLiDatabaseFileOfficial::FindBlankIndex(unsigned short* index) const
{
    unsigned short i;
    for (i = 0; i < 3000; ++i)
    {
        if (FFLiIsNullMiiID(&GetImpl(i)->createId))
        {
            if (index != NULL)
                *index = i;
            return true;
        }
    }
    return false;
}

bool FFLiDatabaseFileOfficial::Set(const FFLiCharInfo& charInfo, unsigned short index)
{
    if (index >= 3000)
        return false;
    FFLiCharInfo2MiiDataOfficial(GetImpl(index), charInfo, false);
    UpdateSaveCount();
    UpdateCrc();
    return true;
}

bool FFLiDatabaseFileOfficial::Restore()
{
    if (!IsValidIdentifier())
        Init();
    else if (!IsValidCrc())
        UpdateCrc();
    return IsValid();
}

void FFLiDatabaseFileOfficial::ResetAllAuthorID(const FFLiAuthorID& authorId)
{
    unsigned short i;
    for (i = 0; i < 3000; ++i)
        if (FFLiIsSameAuthorID(&GetImpl(i)->authorId, &authorId))
            memset(&GetImpl(i)->authorId, 0, sizeof(FFLiAuthorID));
    UpdateSaveCount();
}

void FFLiDatabaseFileOfficial::ResetAuthorID(unsigned short index, const FFLiAuthorID& authorId)
{
    if (index < 3000 && FFLiIsSameAuthorID(&GetImpl(index)->authorId, &authorId))
    {
        memset(&GetImpl(index)->authorId, 0, sizeof(FFLiAuthorID));
        UpdateSaveCount();
    }
}

bool FFLiDatabaseFileOfficial::SetRegular(unsigned short index, bool regular)
{
    unsigned int listIndex;
    if (index >= 3000 || !GetImpl(index))
        return false;
    if (regular)
    {
        if (!FindEmptyRegularList(&listIndex))
            return false;
        m_regular[listIndex] = GetImpl(index)->createId;
    }
    else
        DeleteRegularList(&GetImpl(index)->createId);
    UpdateSaveCount();
    return true;
}

bool FFLiDatabaseFileOfficial::IsRegular(unsigned short index, bool includeTemporary, bool includeSpecial) const
{
    FFLiCharInfo charInfo;
    unsigned int listIndex;
    if (Get(&charInfo, index, includeTemporary, includeSpecial))
        return FindRegularList(&listIndex, &GetImpl(index)->createId);
    return false;
}

short FFLiDatabaseFileOfficial::GetRegularNum() const
{
    short count = 0;
    unsigned int i;
    for (i = 0; i < 50; ++i)
        if (FFLiIsValidMiiID(&m_regular[i]))
            ++count;
    return count;
}

bool FFLiDatabaseFileOfficial::AdjustRegularList(AdjustRegularBuffer* buffer)
{
    unsigned short i;
    unsigned int j;
    bool changed = false;
    buffer->num = 0;
    for (i = 0; i < 3000; ++i)
        if (Get(NULL, i, true, true))
            buffer->createIds[buffer->num++] = GetImpl(i)->createId;
    for (j = 0; j < 50; ++j)
    {
        if (FFLiIsValidMiiID(&m_regular[j]))
        {
            bool exists = false;
            unsigned int k;
            for (k = 0; k < buffer->num; ++k)
                if (FFLiIsSameMiiID(&m_regular[j], &buffer->createIds[k]))
                    exists = true;
            if (!exists)
            {
                SetNullCreateID(m_regular[j]);
                changed = true;
            }
        }
    }
    return changed;
}

void FFLiDatabaseFileOfficial::SwapEndian()
{
    unsigned char* bytes = (unsigned char*)this;
    unsigned int i;
    for (i = 0; i < 0x43840; i += 4)
    {
        unsigned char a = bytes[i];
        bytes[i] = bytes[i + 3];
        bytes[i + 3] = a;
        a = bytes[i + 1];
        bytes[i + 1] = bytes[i + 2];
        bytes[i + 2] = a;
    }
}

unsigned short FFLiDatabaseFileOfficial::Num() const
{
    return 3000;
}
