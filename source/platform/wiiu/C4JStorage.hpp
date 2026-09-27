#ifndef SOURCE_PLATFORM_WIIU_C4JSTORAGE_HPP
#define SOURCE_PLATFORM_WIIU_C4JSTORAGE_HPP

#include <stdint.h>

class C4JStorage
{
public:
    enum PROFILESETTING
    {
        PROFILESETTING_YAXIS_INVERSION = 0,
        PROFILESETTING_CONTROLLER_SENSITIVITY = 1,
        PROFILESETTING_VIBRATION = 2,
        PROFILESETTING_SWAP_STICKS = 3
    };

    struct PROFILESETTINGS
    {
        unsigned int values[3];
        unsigned char flags;
    };

    enum EMessageResult
    {
        EMessageResult_Cancelled = 3,
        EMessageResult_Accept,
        EMessageResult_Decline,
        EMessageResult_ThirdOption,
        EMessageResult_FourthOption
    };

    struct DLC_TMS_DETAILS;
    struct TMSPP_FILEDATA;

    enum eGlobalStorage { eGlobalStorageValue = 0 };
    enum eTMS_FileType { eTMS_FileTypeValue = 0 };
    enum ESavingMessage
    {
        ESavingMessage_0 = 0,
        ESavingMessage_2 = 2
    };
    enum ESaveIncompleteType
    {
        ESaveIncompleteType_0 = 0,
        ESaveIncompleteType_1 = 1,
        ESaveIncompleteType_2 = 2
    };
    enum ESaveGameState
    {
        ESaveGame_Idle,
        ESaveGame_Save,
        ESaveGame_SaveComplete,
        ESaveGame_SaveFailed,
        ESaveGame_SaveCancelled,
        ESaveGame_SaveIncomplete,
        ESaveGame_SaveIncompleteWaiting,
        ESaveGame_Load,
        ESaveGame_LoadComplete,
        ESaveGame_LoadFailed,
        ESaveGame_Delete,
        ESaveGame_DeleteComplete,
        ESaveGame_DeleteFailed,
        ESaveGame_Rename,
        ESaveGame_RenameComplete,
        ESaveGame_RenameFailed,
        ESaveGame_Copy,
        ESaveGame_CopyComplete,
        ESaveGame_CopyFailed,
        ESaveGame_CopyFailedNoSpace,
        ESaveGame_CopyFailedOther,
        ESaveGame_GetThumbnail
    };
    enum eOptionsCallback
    {
        eOptionsCallback_0 = 0,
        eOptionsCallback_1 = 1,
        eOptionsCallback_2 = 2,
        eOptionsCallback_3 = 3,
        eOptionsCallback_4 = 4,
        eOptionsCallback_5 = 5,
        eOptionsCallback_7 = 7,
        eOptionsCallback_9 = 9
    };
    enum eGeneralData_Callback_Status { eGeneralData_Callback_StatusValue = 0 };

    void SetSaveDeviceSelected(uint32_t device, bool selected);
};

#endif
