#include "platform/wiiu/FFLiDatabaseFileHidden.hpp"

#include <string.h>

struct FFLCreateID
{
    unsigned char data[10];
};

void FFLiPutCRC16(unsigned short seed, void* data, unsigned int size);
bool FFLiIsValidCRC16(const void* data, unsigned int size);

void FFLiDatabaseFileHidden::UpdateCrc()
{
    FFLiPutCRC16(0, this, 0xac00);
}

void FFLiDatabaseFileHidden::Init()
{
    m_identifier = 0x46464841;
    m_saveCount = 0xffff;
    m_oldest = 0xffff;
    unsigned short i;
    for (i = 0; i < 500; ++i)
        m_orders[i].Init();
    memset(m_miis, 0, sizeof(m_miis));
    memset(m_extensionData, 0, sizeof(m_extensionData));
    UpdateCrc();
}

bool FFLiDatabaseFileHidden::IsValidCrc() const
{
    return FFLiIsValidCRC16(this, 0xac00);
}

bool FFLiDatabaseFileHidden::IsValidIdentifier() const
{
    return m_identifier == 0x46464841;
}

bool FFLiDatabaseFileHidden::IsValid() const
{
    return IsValidIdentifier() && IsValidCrc();
}

FFLiDatabaseFileHidden::HiddenMiiData* FFLiDatabaseFileHidden::GetImpl(unsigned short index)
{
    return index < 500 ? &m_miis[index] : NULL;
}

const FFLiDatabaseFileHidden::HiddenMiiData* FFLiDatabaseFileHidden::GetImpl(unsigned short index) const
{
    return index < 500 ? &m_miis[index] : NULL;
}

FFLiOrderData* FFLiDatabaseFileHidden::GetOrder(unsigned short index)
{
    return index < 500 ? &m_orders[index] : NULL;
}

const FFLiOrderData* FFLiDatabaseFileHidden::GetOrder(unsigned short index) const
{
    return index < 500 ? &m_orders[index] : NULL;
}

bool FFLiDatabaseFileHidden::FindBlankIndex(unsigned short* index) const
{
    unsigned short i;
    for (i = 0; i < 500; ++i)
    {
        bool blank = true;
        unsigned int j;
        for (j = 0; j < sizeof(m_miis[i].createId); ++j)
            if (m_miis[i].createId[j] != 0) blank = false;
        if (blank)
        {
            if (index != NULL) *index = i;
            return true;
        }
    }
    return false;
}

void FFLiDatabaseFileHidden::ClearOrder(unsigned short index)
{
    if (index < 500)
    {
        unsigned short next = m_orders[index].NextIndex();
        unsigned short previous = m_orders[index].PrevIndex();
        if (next < 500) m_orders[next].SetPrevIndex(previous);
        if (previous < 500) m_orders[previous].SetNextIndex(next);
        if (m_oldest == index) m_oldest = next;
        m_orders[index].Clear();
    }
}

void FFLiDatabaseFileHidden::AppendOrderToTail(unsigned short index)
{
    if (index >= 500) return;
    ClearOrder(index);
    unsigned short tail = m_oldest;
    if (tail == 0xffff)
    {
        m_oldest = index;
        return;
    }
    while (m_orders[tail].NextIndex() < 500) tail = m_orders[tail].NextIndex();
    m_orders[tail].SetNextIndex(index);
    m_orders[index].SetPrevIndex(tail);
}

unsigned short FFLiDatabaseFileHidden::GetOldest() const
{
    return m_oldest;
}

unsigned short FFLiDatabaseFileHidden::NumOfGenderWithIndex(unsigned short* indices, FFLGender gender) const
{
    unsigned short count = 0;
    unsigned short i;
    for (i = 0; i < 500; ++i)
        if (m_miis[i].data[0] == gender)
        {
            if (indices != NULL) indices[count] = i;
            ++count;
        }
    return count;
}

void FFLiDatabaseFileHidden::UpdateMiddleDBRandom(FFLiMiddleDB* middleDb, FFLiAllocator* allocator) const
{
    (void)middleDb;
    (void)allocator;
    unsigned short index = m_oldest;
    while (index < 500) index = m_orders[index].NextIndex();
}

void FFLiDatabaseFileHidden::UpdateMiddleDBTime(FFLiMiddleDB* middleDb, bool ascending) const
{
    (void)middleDb;
    unsigned short index = m_oldest;
    while (index < 500) index = ascending ? m_orders[index].NextIndex() : m_orders[index].PrevIndex();
}

bool FFLiDatabaseFileHidden::Search(unsigned short* index, const FFLCreateID& createId) const
{
    unsigned short i;
    for (i = 0; i < 500; ++i)
        if (memcmp(m_miis[i].createId, createId.data, sizeof(createId.data)) == 0)
        {
            if (index != NULL) *index = i;
            return true;
        }
    return false;
}

void FFLiDatabaseFileHidden::DeleteMii(unsigned short index)
{
    if (index < 500)
    {
        ClearOrder(index);
        memset(&m_miis[index], 0, sizeof(m_miis[index]));
    }
}

void FFLiDatabaseFileHidden::DeleteMii(const FFLCreateID& createId)
{
    unsigned short index;
    if (Search(&index, createId)) DeleteMii(index);
}

bool FFLiDatabaseFileHidden::Add(const FFLiMiiDataCore& data, bool temporary)
{
    unsigned short index;
    if (!FindBlankIndex(&index)) return false;
    memset(&m_miis[index], 0, sizeof(m_miis[index]));
    memcpy(m_miis[index].header, data.header, sizeof(data.header));
    memcpy(m_miis[index].createId, data.createId, sizeof(data.createId));
    memcpy(m_miis[index].authorId, data.authorId, sizeof(data.authorId));
    memcpy(m_miis[index].data, data.data, sizeof(data.data));
    m_miis[index].data[51] = temporary ? 1 : 0;
    AppendOrderToTail(index);
    UpdateCrc();
    return true;
}

bool FFLiDatabaseFileHidden::Restore()
{
    if (!IsValidIdentifier()) Init();
    else if (!IsValidCrc()) UpdateCrc();
    return IsValid();
}

void FFLiDatabaseFileHidden::UpdateMiddleDB(FFLiMiddleDB* middleDb, FFLiAllocator* allocator) const
{
    UpdateMiddleDBRandom(middleDb, allocator);
}

void FFLiDatabaseFileHidden::SwapEndian()
{
    unsigned char* bytes = (unsigned char*)this;
    unsigned int i;
    for (i = 0; i + 3 < 0xac00; i += 4)
    {
        unsigned char a = bytes[i]; bytes[i] = bytes[i + 3]; bytes[i + 3] = a;
        a = bytes[i + 1]; bytes[i + 1] = bytes[i + 2]; bytes[i + 2] = a;
    }
}

unsigned short FFLiDatabaseFileHidden::Num() const
{
    return 500;
}
