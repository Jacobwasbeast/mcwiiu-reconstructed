#ifndef PLATFORM_WIIU_FFLIDATABASEFILEHIDDEN_HPP
#define PLATFORM_WIIU_FFLIDATABASEFILEHIDDEN_HPP

#include "platform/wiiu/FFLiOrderData.hpp"
#include "platform/wiiu/FFLiMiiDataOfficial.hpp"

typedef unsigned char FFLGender;
struct FFLCreateID;
class FFLiAllocator;
struct FFLiMiddleDB;

class FFLiDatabaseFileHidden
{
private:
    struct HiddenMiiData
    {
        unsigned char header[12];
        unsigned char createId[10];
        unsigned char authorId[10];
        unsigned char data[52];
    };

    unsigned int m_identifier;
    unsigned short m_saveCount;
    unsigned short m_oldest;
    FFLiOrderData m_orders[500];
    HiddenMiiData m_miis[500];
    unsigned char m_extensionData[22];
    unsigned short m_crc;

public:
    void UpdateCrc();
    void Init();
    bool IsValidCrc() const;
    bool IsValidIdentifier() const;
    bool IsValid() const;
    HiddenMiiData* GetImpl(unsigned short index);
    const HiddenMiiData* GetImpl(unsigned short index) const;
    FFLiOrderData* GetOrder(unsigned short index);
    const FFLiOrderData* GetOrder(unsigned short index) const;
    bool FindBlankIndex(unsigned short* index) const;
    void ClearOrder(unsigned short index);
    void AppendOrderToTail(unsigned short index);
    unsigned short GetOldest() const;
    unsigned short NumOfGenderWithIndex(unsigned short* indices, FFLGender gender) const;
    void UpdateMiddleDBRandom(FFLiMiddleDB* middleDb, FFLiAllocator* allocator) const;
    void UpdateMiddleDBTime(FFLiMiddleDB* middleDb, bool ascending) const;
    bool Search(unsigned short* index, const FFLCreateID& createId) const;
    void DeleteMii(unsigned short index);
    void DeleteMii(const FFLCreateID& createId);
    bool Add(const FFLiMiiDataCore& data, bool temporary);
    bool Restore();
    void UpdateMiddleDB(FFLiMiddleDB* middleDb, FFLiAllocator* allocator) const;
    void SwapEndian();
    unsigned short Num() const;
};

#endif
