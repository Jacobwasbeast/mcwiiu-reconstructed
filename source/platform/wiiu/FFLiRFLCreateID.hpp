#ifndef PLATFORM_WIIU_FFLIRFLCREATEID_HPP
#define PLATFORM_WIIU_FFLIRFLCREATEID_HPP

#include "platform/wiiu/FFLiTypes.hpp"

struct FFLiRFLCreateID
{
    bool IsValid() const;
    bool IsTemporary() const;
    bool IsValidOnNand() const;
    void Convert(FFLCreateID* createId, const FFLiAuthorID* authorId) const;
    bool IsNormal() const;
    bool IsSpecial() const;
    bool IsDs() const;
    bool IsWii() const;
};

#endif
