#ifndef PLATFORM_WIIU_FFLIDATABASERANDOM_HPP
#define PLATFORM_WIIU_FFLIDATABASERANDOM_HPP

#include "platform/wiiu/FFLiRandomContext.hpp"

typedef unsigned char FFLGender;
typedef unsigned char FFLAge;
typedef unsigned char FFLRace;

struct FFLiCharInfo;
struct FFLiMiddleDB;

class FFLiDatabaseRandom
{
public:
    explicit FFLiDatabaseRandom(FFLiRandomContext* randomContext);
    ~FFLiDatabaseRandom();
    void DetermineParam(FFLGender* gender, FFLAge* age, FFLRace* race);
    void Get(FFLiCharInfo* charInfo, FFLGender gender, FFLAge age, FFLRace race);
    void UpdateMiddleDB(FFLiMiddleDB* middleDB);

private:
    FFLiRandomContext* mRandomContext;
};

#endif
