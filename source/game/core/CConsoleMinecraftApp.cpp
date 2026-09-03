#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include <stdlib.h>
#include <string.h>

extern void altMain();
extern void Sleep(unsigned int);
extern int isMainThread();
extern void ShutdownAndExit();
extern int CreateFileA(const char*, unsigned int, unsigned int, void*, unsigned int, unsigned int, int);
extern int ReadFile(int, void*, unsigned int, unsigned int*, void*);
extern int GetFileSize(int, unsigned int*);
extern int CloseHandle(int);

CConsoleMinecraftApp* theApp = 0;
static unsigned char s_requestBackground = 0;
static unsigned char s_signInRequested = 0;

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
    m_noSpaceForSaveQuota = false;
    m_drcOutputEnabled = true;
    m_drcPreventingSplitscreen = true;
    m_drcCheckPlayerCountEnabled = true;
    m_drcIconEnabled = false;
    m_ignoreMainMenuInputDuringInvite = false;
    m_signInResult = 0;
    m_dlcInfo = 0;
    m_saveThumbnail = 0;
    m_saveThumbnailSize = 0;
    m_productSKU = 0;
    m_commerce = 0;
    theApp = this;
}
CConsoleMinecraftApp::~CConsoleMinecraftApp() { }
bool CConsoleMinecraftApp::IsAmericanSKU() { return m_productSKU == 1; }
uint32_t CConsoleMinecraftApp::GetCommerceInstance() { return (uint32_t)(uintptr_t)m_commerce; }
void CConsoleMinecraftApp::RequestToBackground() { s_requestBackground = 1; }
uint8_t CConsoleMinecraftApp::ReadyToBackground() { return (uint8_t)(s_requestBackground ^ 1); }
uint8_t CConsoleMinecraftApp::GetDRCIconEnabled() { return m_drcIconEnabled; }
void CConsoleMinecraftApp::SetDRCOuputEnabled(bool value) { m_drcOutputEnabled = value; }
uint8_t CConsoleMinecraftApp::GetDRCOutputEnabled() { return m_drcOutputEnabled; }
uint8_t CConsoleMinecraftApp::GetDRCCheckPlayerCountEnabled() { return m_drcCheckPlayerCountEnabled; }
void CConsoleMinecraftApp::FatalLoadError(const wchar_t*, ...)
{
    altMain();
    for (;;) { Sleep(10); if (isMainThread()) ShutdownAndExit(); }
}
void CConsoleMinecraftApp::InitialiseDLCInfo() { m_dlcInfo = new cNintendoDLCInfo(); }
uint32_t CConsoleMinecraftApp::ReadProductCodes()
{
    char productCode[10];
    unsigned int bytesRead = 0;
    memset(productCode, 0, sizeof(productCode));
    int handle = CreateFileA("WiiU_WiiUProductCodes.bin", 0x80000000U, 0, 0, 3, 0x80, 0);
    if (handle == -1) return 0;
    unsigned int count = 0;
    if (GetFileSize(handle, &count) == 0) return 1;
    if (ReadFile(handle, productCode, 9, &bytesRead, 0) == 0) return 0;
    CloseHandle(handle);
    m_productSKU = 0;
    if (strcmp(productCode, "AME01D9D") == 0) m_productSKU = 1;
    else if (strcmp(productCode, "AMJ01DBE") == 0) m_productSKU = 2;
    return 1;
}
uint32_t CConsoleMinecraftApp::GetProductSKU() { return (uint32_t)m_productSKU; }
void CConsoleMinecraftApp::CommerceInit() { m_commerceState = 0; m_commerceError = 0; m_commerceResult = 0; m_commerceProduct = 0; m_commerceCategory = 0; m_commerceInstanceState = 0; }
uint32_t CConsoleMinecraftApp::RequestSignInUIChoices(int value) { s_signInRequested = (unsigned char)(value != 0); return 0; }
uint32_t CConsoleMinecraftApp::DisplaySavingMessageWiiU(void*, C4JStorage::ESavingMessage, int) { return 0; }
void CConsoleMinecraftApp::SetNoSpaceForSaveQuota(bool value) { m_noSpaceForSaveQuota = value; }
uint32_t CConsoleMinecraftApp::Callback_SaveGameIncompleteMessageBoxReturned(void*, int, C4JStorage::EMessageResult) { if (theApp != 0) theApp->m_noSpaceForSaveQuota = false; return 0; }
uint64_t CConsoleMinecraftApp::Callback_SaveGameIncomplete(void* value, C4JStorage::ESaveIncompleteType type)
{
    if (theApp != 0 && (type == 1 || type == 2))
    {
        theApp->EnterSaveNotificationSection();
    }
    return (uint64_t)(uintptr_t)value << 32;
}
uint32_t CConsoleMinecraftApp::GetDLCInfoInstance() { return (uint32_t)(uintptr_t)m_dlcInfo; }
uint64_t CConsoleMinecraftApp::HandleDLCLicenseChange(std::wstring&) { return 0; }
void CConsoleMinecraftApp::SetDRCIconEnabled(bool value) { m_drcIconEnabled = value; }
void CConsoleMinecraftApp::SetRichPresenceContext(int, int) { }
uint8_t CConsoleMinecraftApp::HasNoSpaceForSaveQuota() { return m_noSpaceForSaveQuota; }
void CConsoleMinecraftApp::SetDRCCheckPlayerCountEnabled(bool value) { m_drcCheckPlayerCountEnabled = value; }
uint8_t CConsoleMinecraftApp::getIgnoreMainMenuInputDuringInvite() { return m_ignoreMainMenuInputDuringInvite; }
void CConsoleMinecraftApp::ReleaseSaveThumbnail() { if (m_saveThumbnail != 0) { free(m_saveThumbnail); m_saveThumbnail = 0; m_saveThumbnailSize = 0; } }
void CConsoleMinecraftApp::ExitGame() { }
void CConsoleMinecraftApp::StoreLaunchData() { }
void CConsoleMinecraftApp::CaptureSaveThumbnail() { }
void CConsoleMinecraftApp::GetSaveThumbnail(unsigned char** data, unsigned int* size)
{
    if (m_saveThumbnail != 0)
    {
        if (data != 0)
        {
            *data = new unsigned char[m_saveThumbnailSize];
            *size = m_saveThumbnailSize;
            memcpy(*data, m_saveThumbnail, m_saveThumbnailSize);
        }
        free(m_saveThumbnail);
        m_saveThumbnail = 0;
    }
    else if (data != 0)
    {
        unsigned char* thumbnail = 0;
        unsigned int thumbnailSize = 0;
        GetScreenshot(0, &thumbnail, &thumbnailSize);
        *data = new unsigned char[thumbnailSize];
        *size = thumbnailSize;
        memcpy(*data, thumbnail, thumbnailSize);
    }
}
void CConsoleMinecraftApp::GetScreenshot(int, unsigned char**, unsigned int*) { }
uint32_t CConsoleMinecraftApp::LoadLocalTMSFile(wchar_t*) { return 0xffffffffU; }
uint32_t CConsoleMinecraftApp::LoadLocalTMSFile(wchar_t*, eFileExtensionType) { return 0xffffffffU; }
void CConsoleMinecraftApp::FreeLocalTMSFiles(eTMSFileType) { }
uint32_t CConsoleMinecraftApp::GetLocalTMSFileIndex(wchar_t*, bool, eFileExtensionType) { return 0xffffffffU; }
void CConsoleMinecraftApp::TemporaryCreateGameStart() { }
uint32_t CConsoleMinecraftApp::UpgradeTrial() { if (m_commerceState == 1) { m_commerceState = 20; return 1; } return 0; }
void CConsoleMinecraftApp::RequestSignInUIWiiUReturnedSelected(MiiDataEntry*, int, int) { m_signInResult = 0; s_signInRequested = 0; }
uint8_t CConsoleMinecraftApp::IsDrcPreventingSplitscreen() { return m_drcPreventingSplitscreen; }
void CConsoleMinecraftApp::setIgnoreMainMenuInputDuringInvite(bool value) { m_ignoreMainMenuInputDuringInvite = value; }
void CConsoleMinecraftApp::LoadLocalDLCImages() { if (m_dlcInfo != 0) m_dlcInfo->LoadLocalDLCImages(); }
void CConsoleMinecraftApp::FreeLocalDLCImages() { if (m_dlcInfo != 0) m_dlcInfo->FreeLocalDLCImages(); }
void CConsoleMinecraftApp::ReadBannedList(int, eTMSAction, bool) { }
uint32_t CConsoleMinecraftApp::LoadLocalTMSFile(char*) { return 0xffffffffU; }
uint32_t CConsoleMinecraftApp::GetProductCode() { return (uint32_t)m_productSKU; }
const wchar_t* CConsoleMinecraftApp::GetSaveFolderPrefix() { return L""; }
const char* CConsoleMinecraftApp::GetCommerceCategory() { return ""; }
const char* CConsoleMinecraftApp::GetTexturePacksCategoryID() { return ""; }
const char* CConsoleMinecraftApp::GetUpgradeKey() { return ""; }
bool CConsoleMinecraftApp::IsJapaneseSKU() { return m_productSKU == 2; }
bool CConsoleMinecraftApp::IsEuropeanSKU() { return m_productSKU == 0; }
bool CConsoleMinecraftApp::IsChinaSKU() { return m_productSKU == 3; }
void CConsoleMinecraftApp::SystemServiceTick() { ++m_commerceInstanceState; }
void CConsoleMinecraftApp::SaveDataDialogTick() { if (m_noSpaceForSaveQuota) m_commerceError = 1; }
void CConsoleMinecraftApp::CommerceTick() { if (m_commerceState != 0) ++m_commerceState; }
uint32_t CConsoleMinecraftApp::GetCommerceState() { return (uint32_t)m_commerceState; }
uint32_t CConsoleMinecraftApp::Checkout(char* product) { if (product == 0) return 0; m_commerceProduct = 1; return 1; }
uint32_t CConsoleMinecraftApp::DownloadAlreadyPurchased(char* product) { if (product == 0) return 0; m_commerceProduct = 2; return 1; }
bool CConsoleMinecraftApp::DLCAlreadyPurchased(char* product) { return product != 0 && m_commerceProduct != 0; }
uint32_t CConsoleMinecraftApp::CommerceInitCallback(void*, int result) { if (theApp != 0) theApp->m_commerceResult = result; return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::CommerceGetCategoriesCallback(void*, int result) { return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::CommerceGetProductListCallback(void*, int result) { return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::CommerceAddDetailedProductInfoCallback(void*, int result) { return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::CommerceCheckoutCallback(void*, int result) { if (theApp != 0) theApp->m_commerceResult = result; return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::CheckoutSessionStartedCallback(void*, int result) { return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::DownloadAlreadyPurchasedSessionStartedCallback(void*, int result) { return (uint32_t)result; }
uint32_t CConsoleMinecraftApp::UpgradeTrialSessionStartedCallback(void*, int result) { return (uint32_t)result; }
void CConsoleMinecraftApp::ShowPatchAvailableError() { m_commerceError = 1; }
void CConsoleMinecraftApp::PatchAvailableDialogTick() { if (m_commerceError != 0) m_commerceError = 0; }
