#ifndef GAME_CORE_CCONSOLEMINECRAFTAPP_HPP
#define GAME_CORE_CCONSOLEMINECRAFTAPP_HPP

#include "game/core/CMinecraftApp.hpp"
#include <stdint.h>
#include <string>

class ConsoleUIController;
class cNintendoDLCInfo;
class cCommerceNintendoWiiU;
struct MiiDataEntry;
enum eFileExtensionType { eFileExtensionType_Default = 0 };
enum eTMSFileType { eTMSFileType_Default = 0 };
enum eTMSAction { eTMSAction_Default = 0 };
namespace C4JStorage { enum ESavingMessage { ESavingMessage_Default = 0 }; enum EMessageResult { EMessageResult_Default = 0 }; enum ESaveIncompleteType { ESaveIncompleteType_Default = 0 }; }

class CConsoleMinecraftApp : public CMinecraftApp
{
public:
    CConsoleMinecraftApp();
    virtual ~CConsoleMinecraftApp();
    bool IsAmericanSKU();
    uint32_t GetCommerceInstance();
    static void RequestToBackground();
    static uint8_t ReadyToBackground();
    uint8_t GetDRCIconEnabled();
    void SetDRCOuputEnabled(bool);
    uint8_t GetDRCOutputEnabled();
    uint8_t GetDRCCheckPlayerCountEnabled();
    void FatalLoadError(const wchar_t*, ...);
    void InitialiseDLCInfo();
    uint32_t ReadProductCodes();
    uint32_t GetProductSKU();
    void CommerceInit();
    static uint32_t RequestSignInUIChoices(int);
    static uint32_t DisplaySavingMessageWiiU(void*, C4JStorage::ESavingMessage, int);
    void SetNoSpaceForSaveQuota(bool);
    static uint32_t Callback_SaveGameIncompleteMessageBoxReturned(void*, int, C4JStorage::EMessageResult);
    static uint64_t Callback_SaveGameIncomplete(void*, C4JStorage::ESaveIncompleteType);
    uint32_t GetDLCInfoInstance();
    uint64_t HandleDLCLicenseChange(std::wstring&);
    void SetDRCIconEnabled(bool);
    void SetRichPresenceContext(int, int);
    uint8_t HasNoSpaceForSaveQuota();
    void SetDRCCheckPlayerCountEnabled(bool);
    uint8_t getIgnoreMainMenuInputDuringInvite();
    void ReleaseSaveThumbnail();
    void ExitGame();
    void StoreLaunchData();
    void CaptureSaveThumbnail();
    void GetSaveThumbnail(unsigned char**, unsigned int*);
    void GetScreenshot(int, unsigned char**, unsigned int*);
    uint32_t LoadLocalTMSFile(wchar_t*);
    uint32_t LoadLocalTMSFile(wchar_t*, eFileExtensionType);
    void FreeLocalTMSFiles(eTMSFileType);
    uint32_t GetLocalTMSFileIndex(wchar_t*, bool, eFileExtensionType);
    void TemporaryCreateGameStart();
    uint32_t UpgradeTrial();
    void RequestSignInUIWiiUReturnedSelected(MiiDataEntry*, int, int);
    uint8_t IsDrcPreventingSplitscreen();
    void setIgnoreMainMenuInputDuringInvite(bool);
    void LoadLocalDLCImages();
    void FreeLocalDLCImages();
    void ReadBannedList(int, eTMSAction, bool);
    uint32_t LoadLocalTMSFile(char*);
    uint32_t GetProductCode();
    const wchar_t* GetSaveFolderPrefix();
    const char* GetCommerceCategory();
    const char* GetTexturePacksCategoryID();
    const char* GetUpgradeKey();
    bool IsJapaneseSKU();
    bool IsEuropeanSKU();
    bool IsChinaSKU();
    void SystemServiceTick();
    void SaveDataDialogTick();
    void CommerceTick();
    uint32_t GetCommerceState();
    uint32_t Checkout(char*);
    uint32_t DownloadAlreadyPurchased(char*);
    bool DLCAlreadyPurchased(char*);
    static uint32_t CommerceInitCallback(void*, int);
    static uint32_t CommerceGetCategoriesCallback(void*, int);
    static uint32_t CommerceGetProductListCallback(void*, int);
    static uint32_t CommerceAddDetailedProductInfoCallback(void*, int);
    static uint32_t CommerceCheckoutCallback(void*, int);
    static uint32_t CheckoutSessionStartedCallback(void*, int);
    static uint32_t DownloadAlreadyPurchasedSessionStartedCallback(void*, int);
    static uint32_t UpgradeTrialSessionStartedCallback(void*, int);
    void ShowPatchAvailableError();
    void PatchAvailableDialogTick();
private:
    unsigned char m_thumbnailHeader[4];
    unsigned char* m_saveThumbnail;
    unsigned int m_saveThumbnailSize;
    unsigned char m_signInReserved[12];
    unsigned char m_signInResult;
    unsigned char m_dlcReserved[3];
    cNintendoDLCInfo* m_dlcInfo;
    bool m_ignoreMainMenuInputDuringInvite;
    unsigned char m_drcReserved[3];
    unsigned char m_drcIconEnabled;
    unsigned char m_commerceReserved[51];
    int m_commerceState;
    int m_commerceError;
    int m_commerceResult;
    int m_commerceProduct;
    int m_commerceCategory;
    int m_commerceInstanceState;
    bool m_noSpaceForSaveQuota;
    bool m_drcOutputEnabled;
    bool m_drcPreventingSplitscreen;
    bool m_drcCheckPlayerCountEnabled;
    unsigned char m_productReserved[12];
    int m_productSKU;
    unsigned char m_instanceReserved[8];
    int m_commerce;
};

extern CConsoleMinecraftApp* theApp;

#endif
