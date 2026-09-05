#ifndef PLATFORM_WIIU_FFLIMIDDLEDBRANDOMPARAM_HPP
#define PLATFORM_WIIU_FFLIMIDDLEDBRANDOMPARAM_HPP

typedef unsigned char FFLGender;
typedef unsigned char FFLAge;
typedef unsigned char FFLRace;

struct FFLiMiddleDBRandomParam
{
    FFLGender mGender;
    FFLAge mAge;
    FFLRace mRace;

    void Init();
    void Set(FFLGender gender, FFLAge age, FFLRace race);
};

#endif
