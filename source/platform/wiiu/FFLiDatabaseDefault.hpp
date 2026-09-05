#ifndef PLATFORM_WIIU_FFLIDATABASEDEFAULT_HPP
#define PLATFORM_WIIU_FFLIDATABASEDEFAULT_HPP

#include "platform/wiiu/FFLiTypes.hpp"
#include "platform/wiiu/FFLiMiiDataOfficial.hpp"

struct FFLiMiiDataCoreRFL
{
    unsigned short data[27];
};

struct FFLiDatabaseDefault
{
    FFLiMiiDataOfficial m_elements[6];

    FFLiDatabaseDefault();
    ~FFLiDatabaseDefault();
    static void InitElement(FFLiMiiDataOfficial* element, const FFLiMiiDataCoreRFL* source, unsigned int index);
    bool Get(FFLiCharInfo* charInfo, unsigned int index) const;
    bool Search(unsigned short* index, const FFLCreateID* createId) const;
    unsigned short Num() const;
};

#endif
