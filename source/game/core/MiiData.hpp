#ifndef GAME_CORE_MIIDATA_HPP
#define GAME_CORE_MIIDATA_HPP

#include <vector>

#include "game/util/ACTUuid.hpp"

struct MiiDataEntry
{
    int mIconTexture;
    void* mIconMemory;
    unsigned short mName[11];
    unsigned char mCreatorId[10];
    bool mIsAccountMii;
    bool mIsGuestMii;
    ACTUuid mUuid;
};

class MiiData
{
public:
    void LoadAllData();
    void FreeAllData();

private:
    void CreateSysUserData(int slot);
    void CreateGuestData(int index);
    void CreateMiiMakerData(int index);

    std::vector<MiiDataEntry> mSysUserMiis;
    std::vector<MiiDataEntry> mGuestMiis;
    std::vector<MiiDataEntry> mMiiMakerMiis;
};

#endif
