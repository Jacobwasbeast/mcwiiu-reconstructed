#include "vendor/__N_23_FFLiDatabaseManager_cpp_875c9ca8.hpp"

struct FFLiCharInfo
{
    unsigned char miiData[180];
    unsigned short name[11];
    unsigned short creatorName[11];
};

void __N_23_FFLiDatabaseManager_cpp_875c9ca8::ReplaceNameIfAccord(unsigned short* name, unsigned int maximumLength)
{
    unsigned int index = 0;
    if (maximumLength == 0)
    {
        return;
    }
    while (name[index] != 0x40 && name[index] != 0xff20)
    {
        ++index;
        if (maximumLength <= index)
        {
            return;
        }
    }
    name[index] = 0;
}

void __N_23_FFLiDatabaseManager_cpp_875c9ca8::ReplaceNameAndCreatorIfAccord(FFLiCharInfo* charInfo)
{
    ReplaceNameIfAccord(charInfo->name, 11);
    ReplaceNameIfAccord(charInfo->creatorName, 11);
}
