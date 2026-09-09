#include "platform/wiiu/MiiData.hpp"

#include "game/client/render/Renderer.hpp"

#include <string.h>
#include <nn/act/client_cpp.h>

struct FFLColor
{
    float r;
    float g;
    float b;
    float a;
};

struct FFLAdditionalInfo
{
    unsigned short name[11];
    unsigned short creatorName[11];
    FFLCreateID createId;
    FFLColor skinColor;
    unsigned int flags;
    unsigned char faceType;
    unsigned char faceLine;
    unsigned char faceTexture;
    unsigned char hairType;
};

int FFLGetAdditionalInfo(FFLAdditionalInfo* additionalInfo, FFLDataSource dataSource, FFLStoreData* storeData,
                         unsigned short index, int checkFontRegion);
int FFLGetAvailableOfficialDataNum();
bool FFLIsAvailableOfficialData(unsigned short index);
bool FFLIsRegularOfficialData(unsigned short index);

static int sAdditionalInfoResult;

void MiiData::CreateSysUserData(int slot)
{
    FFLStoreData storeData;
    FFLAdditionalInfo additionalInfo;
    MiiDataEntry entry;

    nn::act::GetMiiEx(&storeData, slot);

    sAdditionalInfoResult = FFLGetAdditionalInfo(&additionalInfo, FFL_DATA_SOURCE_STORE_DATA, &storeData, 0, 1);

    memcpy(entry.mName, additionalInfo.name, sizeof(entry.mName));
    memcpy(entry.mCreatorId, &additionalInfo.createId, sizeof(entry.mCreatorId));

    entry.mIsAccountMii = true;
    entry.mIsGuestMii = false;

    nn::act::GetUuidEx(reinterpret_cast<char*>(&entry.mUuid), slot);

    entry.mIconMemory = (void*)gRenderer->CreateMiiIconTexture(FFL_DATA_SOURCE_STORE_DATA, &storeData, 0,
                                                               &entry.mIconTexture);

    mSysUserMiis.push_back(entry);
}

void MiiData::CreateGuestData(int index)
{
    FFLAdditionalInfo additionalInfo;
    MiiDataEntry entry;

    FFLGetAdditionalInfo(&additionalInfo, FFL_DATA_SOURCE_DEFAULT, NULL, index, 1);

    memcpy(entry.mName, additionalInfo.name, sizeof(entry.mName));
    memcpy(entry.mCreatorId, &additionalInfo.createId, sizeof(entry.mCreatorId));

    entry.mIsAccountMii = false;
    entry.mIsGuestMii = true;

    entry.mIconMemory = (void*)gRenderer->CreateMiiIconTexture(FFL_DATA_SOURCE_DEFAULT, NULL, index,
                                                               &entry.mIconTexture);

    mGuestMiis.push_back(entry);
}

void MiiData::CreateMiiMakerData(int index)
{
    FFLAdditionalInfo additionalInfo;
    MiiDataEntry entry;

    FFLGetAdditionalInfo(&additionalInfo, FFL_DATA_SOURCE_OFFICIAL, NULL, index, 1);

    memcpy(entry.mName, additionalInfo.name, sizeof(entry.mName));
    memcpy(entry.mCreatorId, &additionalInfo.createId, sizeof(entry.mCreatorId));

    entry.mIsAccountMii = false;
    entry.mIsGuestMii = false;

    entry.mIconMemory = (void*)gRenderer->CreateMiiIconTexture(FFL_DATA_SOURCE_OFFICIAL, NULL, index,
                                                               &entry.mIconTexture);

    mMiiMakerMiis.push_back(entry);
}

void MiiData::LoadAllData()
{
    if (!gRenderer->InitFFL())
    {
        return;
    }

    int accountCount = nn::act::GetNumOfAccounts();

    for (int slot = 1; slot <= accountCount; ++slot)
    {
        if (nn::act::IsSlotOccupied(slot))
        {
            CreateSysUserData(slot);
        }
    }

    int officialDataCount = FFLGetAvailableOfficialDataNum();
    int foundCount = 0;

    for (int index = 0; index < 3000; ++index)
    {
        if (FFLIsAvailableOfficialData(index))
        {
            if (FFLIsRegularOfficialData(index))
            {
                CreateMiiMakerData(index);
            }

            ++foundCount;

            if (foundCount >= officialDataCount)
            {
                break;
            }
        }
    }

    for (int guestIndex = 0; guestIndex < 6; ++guestIndex)
    {
        CreateGuestData(guestIndex);
    }

    gRenderer->ShutdownFFL();
}

void MiiData::FreeAllData()
{
    for (unsigned int i = 0; i < mSysUserMiis.size(); ++i)
    {
        gRenderer->FreeMiiIconTexture(mSysUserMiis[i].mIconTexture, mSysUserMiis[i].mIconMemory);
    }

    mSysUserMiis.clear();

    for (unsigned int i = 0; i < mGuestMiis.size(); ++i)
    {
        gRenderer->FreeMiiIconTexture(mGuestMiis[i].mIconTexture, mGuestMiis[i].mIconMemory);
    }

    mGuestMiis.clear();

    for (unsigned int i = 0; i < mMiiMakerMiis.size(); ++i)
    {
        gRenderer->FreeMiiIconTexture(mMiiMakerMiis[i].mIconTexture, mMiiMakerMiis[i].mIconMemory);
    }

    mMiiMakerMiis.clear();
}
