#ifndef PLATFORM_WIIU_FFLIMIDDLEDB_HPP
#define PLATFORM_WIIU_FFLIMIDDLEDB_HPP

typedef unsigned char FFLGender;
typedef unsigned char FFLAge;
typedef unsigned char FFLRace;
enum FFLMiddleDBType
{
    FFL_MIDDLE_DB_HIDDEN = 0,
    FFL_MIDDLE_DB_RANDOM = 1,
    FFL_MIDDLE_DB_NET = 2
};

struct FFLiCharInfo;
struct FFLiMiiDataHidden;
struct FFLiMiiDataOfficial;
struct FFLiMiddleDBHiddenParam;
struct FFLiMiddleDBRandomParam;
struct FFLiMiddleDBNetParam;

class FFLiMiddleDB
{
public:
    static unsigned short GetBufferSize(unsigned short count);
    void Init(FFLMiddleDBType type, void* buffer, unsigned short size);
    void SetHiddenParam(FFLGender gender);
    void SetRandomParam(FFLGender gender, FFLAge age, FFLRace race);
    unsigned short Size() const;
    FFLMiddleDBType Type() const;
    void ClearData();
    void SetNetParam(unsigned short index);
    const FFLiMiddleDBHiddenParam* HiddenParam() const;
    const FFLiMiddleDBRandomParam* RandomParam() const;
    const FFLiMiddleDBNetParam* NetParam() const;
    bool IsFull() const;
    void Add(const FFLiCharInfo& charInfo);
    void Add(const FFLiMiiDataHidden& miiData);
    FFLiMiiDataOfficial* Get(unsigned short index);
    const FFLiMiiDataOfficial* Get(unsigned short index) const;
    unsigned short StoredSize() const;
    bool GetCharInfo(FFLiCharInfo* charInfo, unsigned short index) const;

private:
    unsigned char mParameters[8];
    void* mData;
    FFLMiddleDBType mType;
    unsigned short mStoredSize;
};

#endif
