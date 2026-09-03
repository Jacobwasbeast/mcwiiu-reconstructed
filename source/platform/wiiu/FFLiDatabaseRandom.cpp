#include "platform/wiiu/FFLiDatabaseRandom.hpp"

FFLiDatabaseRandom::FFLiDatabaseRandom(FFLiRandomContext* randomContext)
    : mRandomContext(randomContext)
{
}

FFLiDatabaseRandom::~FFLiDatabaseRandom()
{
}

void FFLiDatabaseRandom::DetermineParam(FFLGender* gender, FFLAge* age, FFLRace* race)
{
    if (mRandomContext == 0)
    {
        return;
    }
    if (gender != 0)
    {
        *gender = static_cast<FFLGender>(mRandomContext->Random(2));
    }
    if (age != 0)
    {
        *age = static_cast<FFLAge>(mRandomContext->Random(3));
    }
    if (race != 0)
    {
        *race = static_cast<FFLRace>(mRandomContext->Random(2));
    }
}

void FFLiDatabaseRandom::Get(FFLiCharInfo* charInfo, FFLGender gender, FFLAge age, FFLRace race)
{
    FFLGender selectedGender = gender;
    FFLAge selectedAge = age;
    FFLRace selectedRace = race;
    if (charInfo != 0)
    {
        DetermineParam(&selectedGender, &selectedAge, &selectedRace);
    }
}

void FFLiDatabaseRandom::UpdateMiddleDB(FFLiMiddleDB* middleDB)
{
    FFLGender gender = 0;
    FFLAge age = 0;
    FFLRace race = 0;
    if (middleDB != 0)
    {
        DetermineParam(&gender, &age, &race);
    }
}
