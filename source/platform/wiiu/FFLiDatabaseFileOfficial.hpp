#ifndef PLATFORM_WIIU_FFLIDATABASEFILEOFFICIAL_HPP
#define PLATFORM_WIIU_FFLIDATABASEFILEOFFICIAL_HPP

struct FFLCreateID
{
    unsigned char data[10];
};

struct FFLiAuthorID
{
    unsigned char data[10];
};

struct FFLiCharInfo
{
    unsigned char data[288];
};

struct FFLiMiiDataOfficial
{
    unsigned char header[12];
    FFLCreateID createId;
    FFLiAuthorID authorId;
    unsigned char data[60];
};

class FFLiDatabaseFileOfficial
{
public:
    struct AdjustRegularBuffer
    {
        unsigned int num;
        FFLCreateID createIds[3000];
    };

    FFLiMiiDataOfficial* GetImpl(unsigned short index);
    const FFLiMiiDataOfficial* GetImpl(unsigned short index) const;
    bool FindRegularList(unsigned int* index, const FFLCreateID* createId) const;
    void DeleteRegularList(const FFLCreateID* createId);
    bool FindEmptyRegularList(unsigned int* index) const;
    void UpdateCrc();
    void Init();
    void UpdateSaveCount();
    bool IsValidCrc() const;
    bool IsValidIdentifier() const;
    bool IsValid() const;
    bool Get(FFLiCharInfo* charInfo, unsigned short index, bool includeTemporary, bool includeSpecial) const;
    bool IsAvailable(unsigned short index, bool includeTemporary, bool includeSpecial) const;
    short NumOfAvailable(bool includeTemporary, bool includeSpecial) const;
    bool Search(unsigned short* index, const FFLCreateID& createId) const;
    void DeleteMii(FFLiMiiDataOfficial* miiData, unsigned short index);
    bool FindBlankIndex(unsigned short* index) const;
    bool Set(const FFLiCharInfo& charInfo, unsigned short index);
    bool Restore();
    void ResetAllAuthorID(const FFLiAuthorID& authorId);
    void ResetAuthorID(unsigned short index, const FFLiAuthorID& authorId);
    bool SetRegular(unsigned short index, bool regular);
    bool IsRegular(unsigned short index, bool includeTemporary, bool includeSpecial) const;
    short GetRegularNum() const;
    bool AdjustRegularList(AdjustRegularBuffer* buffer);
    void SwapEndian();
    unsigned short Num() const;

private:
    unsigned int m_identifier;
    unsigned int m_saveCount;
    FFLiMiiDataOfficial m_miis[3000];
    FFLCreateID m_regular[50];
    unsigned char m_extensionData[34];
    unsigned short m_crc;
};

#endif
