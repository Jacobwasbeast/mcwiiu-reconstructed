#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCPack.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include <stdlib.h>
#include <string.h>

extern void altMain();
extern void Sleep(unsigned int);
extern void ShutdownAndExit();
extern unsigned int GetLastFileErrorCodeForViewer();
extern unsigned char g_fatalLoadErrorRequested;
extern unsigned char g_fatalLoadErrorReady;
extern unsigned char g_fatalLoadErrorActive;
extern int CreateFileA(const char*, unsigned int, unsigned int, void*, unsigned int, unsigned int, int);
extern int ReadFile(int, void*, unsigned int, unsigned int*, void*);
extern int GetFileSize(int, unsigned int*);
extern int CloseHandle(int);
extern void SYSLaunchMenu();
extern ConsoleUIController g_ConsoleUIController;
extern CStorage* gStorage;

static unsigned char s_requestBackground = 0;
static unsigned char s_signInRequested = 0;

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
    memset(m_productCode, 0, 4);
    m_noSpaceForSaveQuota = false;
    m_drcCheckPlayerCountEnabled = true;
    m_signInResult = 0;
    m_drcOutputEnabled = true;
    m_drcIconEnabled = false;
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
void CConsoleMinecraftApp::FatalLoadError(const wchar_t* message, ...)
{
    gRenderer->BeginConditionalSurvey((int)message);
    g_pInput->RequestErrorViewer(1, GetLastFileErrorCodeForViewer(), NULL, NULL);
    altMain();
    for (;;)
    {
        if (g_fatalLoadErrorRequested != 0)
            g_fatalLoadErrorReady = 1;
        Sleep(10);
        if (g_fatalLoadErrorActive != 0 && C4JThread::isMainThread())
            ShutdownAndExit();
    }
}
void CConsoleMinecraftApp::InitialiseDLCInfo() { m_dlcInfo = new cNintendoDLCInfo(); }
uint32_t CConsoleMinecraftApp::ReadProductCodes()
{
    unsigned int bytesRead = 0;
    int handle = CreateFileA("WiiU_WiiUProductCodes.bin", 0x80000000U, 0, 0, 3, 0x80, 0);
    if (handle == -1)
        return 0;

    unsigned int fileSize = 0;
    if (GetFileSize(handle, &fileSize) != 0)
    {
        if (ReadFile(handle, m_productCode, 9, &bytesRead, 0) == 0)
            return 0;

        unsigned int dlcCount;
        if (ReadFile(handle, &dlcCount, 4, &bytesRead, 0) == 0)
            return 0;

        for (unsigned int index = 0; index < dlcCount; ++index)
        {
            DLC_INFO* dlcInfo = new DLC_INFO;
            memset(dlcInfo, 0, sizeof(DLC_INFO));

            unsigned int stringLength;
            if (ReadFile(handle, &dlcInfo->ContentType, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->PackType, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->CatalogIndex, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &stringLength, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, dlcInfo->PackName, stringLength, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &stringLength, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, dlcInfo->BannerName, stringLength, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->PackID, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, &dlcInfo->SortIndex, 4, &bytesRead, 0) == 0)
                return 0;
            if (ReadFile(handle, dlcInfo->RomName, 5, &bytesRead, 0) == 0)
                return 0;

            m_dlcInfo->AddDLCInfo(dlcInfo);
        }
        CloseHandle(handle);
    }

    if (strcmp(m_productCode, "AUMP01D75") == 0)
        m_productSKU = 0;
    else if (strcmp(m_productCode, "AUME01D9D") == 0)
        m_productSKU = 1;
    else if (strcmp(m_productCode, "AUMJ01DBE") == 0)
        m_productSKU = 2;
    return 1;
}
uint32_t CConsoleMinecraftApp::GetProductSKU() { return (uint32_t)m_productSKU; }
void CConsoleMinecraftApp::CommerceInit()
{
    m_commerceInitialized = 0;
    m_commerce = new cCommerceNintendoWiiU();
    m_commerceResult = 0;
    m_commerceProduct = 0;
    m_commerceCategory = 0;
    m_commerceError = 0;
    m_commerceState = 0;
    m_commerceInstanceState = 0;
    memset(m_commerceReserved, 0, sizeof(m_commerceReserved));
}
int CConsoleMinecraftApp::RequestSignInUIChoices(int playerId)
{
    g_ConsoleUIController.NavigateToScene(playerId, (EUIScene)0x50, NULL, (EUILayer)6, (EUIGroup)7);
    s_signInRequested = 1;
    return 0;
}
int CConsoleMinecraftApp::DisplaySavingMessageWiiU(void*, C4JStorage::ESavingMessage message, int playerId)
{
    if (s_requestBackground != 0 && message == C4JStorage::ESavingMessage_0)
        s_requestBackground = 0;
    g_ConsoleUIController.ShowSavingMessage((unsigned int)playerId, message);
    return 0;
}
void CConsoleMinecraftApp::SetNoSpaceForSaveQuota(bool value) { m_noSpaceForSaveQuota = value; }
int CConsoleMinecraftApp::Callback_SaveGameIncompleteMessageBoxReturned(void* context, int playerId,
                                                                         C4JStorage::EMessageResult result)
{
    CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
    if (result < C4JStorage::EMessageResult_3)
        return 0;
    if (result == C4JStorage::EMessageResult_4)
    {
        app->LeaveSaveNotificationSection();
        gStorage->ContinueIncompleteOperation();
    }
    else if (result == C4JStorage::EMessageResult_3 || result == C4JStorage::EMessageResult_5)
    {
        app->LeaveSaveNotificationSection();
        gStorage->CancelIncompleteOperation();
    }
    else if (result == C4JStorage::EMessageResult_6)
    {
        g_ConsoleUIController.NavigateToScene(playerId, (EUIScene)0x4e, NULL, (EUILayer)2, (EUIGroup)1);
    }
    return 0;
}
void CConsoleMinecraftApp::Callback_SaveGameIncomplete(void* context, C4JStorage::ESaveIncompleteType type)
{
    if (type == C4JStorage::ESaveIncompleteType_1 || type == C4JStorage::ESaveIncompleteType_2)
    {
        CMinecraftApp* app = static_cast<CMinecraftApp*>(context);
        app->EnterSaveNotificationSection();
        int messageParameters[] = { (int)0xaed18de8, (int)0xce9c596c, (int)0xfec9f75f };
        if (g_ConsoleUIController.RequestErrorMessage(0xe6a7b0a3, 0x92727461, messageParameters, 3, 0xff,
                                                       Callback_SaveGameIncompleteMessageBoxReturned, context, NULL,
                                                       true) != 1)
            return;
    }
    Callback_SaveGameIncompleteMessageBoxReturned(context, 0, C4JStorage::EMessageResult_3);
}
uint32_t CConsoleMinecraftApp::GetDLCInfoInstance() { return (uint32_t)(uintptr_t)m_dlcInfo; }
void CConsoleMinecraftApp::HandleDLCLicenseChange(std::wstring& packName)
{
    DLCPack* pack = theApp.mDLCManager.getPack(packName);
    if (pack != NULL)
        pack->updateLicenseMask(1);
}
void CConsoleMinecraftApp::SetDRCIconEnabled(bool value) { m_drcIconEnabled = value; }
void CConsoleMinecraftApp::SetRichPresenceContext(int context, int value)
{
    gProfile->SetRichPresenceContextValue(context, 0, value);
}
uint8_t CConsoleMinecraftApp::HasNoSpaceForSaveQuota() { return m_noSpaceForSaveQuota; }
void CConsoleMinecraftApp::SetDRCCheckPlayerCountEnabled(bool value) { m_drcCheckPlayerCountEnabled = value; }
uint8_t CConsoleMinecraftApp::getIgnoreMainMenuInputDuringInvite() { return m_ignoreMainMenuInputDuringInvite; }
void CConsoleMinecraftApp::ReleaseSaveThumbnail()
{
    if (m_saveThumbnail.mData != NULL)
    {
        free(m_saveThumbnail.mData);
        m_saveThumbnail.mData = NULL;
    }
}
void CConsoleMinecraftApp::ExitGame() { SYSLaunchMenu(); }
void CConsoleMinecraftApp::StoreLaunchData() { }
void CConsoleMinecraftApp::CaptureSaveThumbnail() { gRenderer->CaptureThumbnail(&m_saveThumbnail); }
void CConsoleMinecraftApp::GetSaveThumbnail(unsigned char** data, unsigned int* size)
{
    if (m_saveThumbnail.mData != NULL)
    {
        if (data != NULL)
        {
            *data = new unsigned char[m_saveThumbnail.mSize];
            *size = m_saveThumbnail.mSize;
            memcpy(*data, m_saveThumbnail.mData, m_saveThumbnail.mSize);
        }
        free(m_saveThumbnail.mData);
        m_saveThumbnail.mData = NULL;
    }
    else if (data != NULL)
    {
        unsigned char* thumbnail = 0;
        unsigned int thumbnailSize = 0;
        gStorage->GetDefaultSaveThumbnail(&thumbnail, &thumbnailSize);
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
uint32_t CConsoleMinecraftApp::UpgradeTrial()
{
    if (m_commerceState == 1)
    {
        m_commerceState = 20;
        return 1;
    }
    if (m_commerceState == 2)
    {
        int messageParameter = 0xb9a1bf4d;
        g_ConsoleUIController.RequestErrorMessage(0xa561a8df, 0x84f3e8d2, &messageParameter, 1,
                                                   gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return 1;
    }
    return 0;
}
void CConsoleMinecraftApp::RequestSignInUIWiiUReturnedSelected(MiiDataEntry* mii, int choice, int pad)
{
    if (mii == NULL)
    {
        gProfile->SetSignInChoicesComplete(false, choice, pad);
    }
    else
    {
        gProfile->SignInUser(mii, pad);
        gProfile->SetSignInChoicesComplete(true, choice, pad);
    }
    m_signInResult = 0;
}
uint8_t CConsoleMinecraftApp::IsDrcPreventingSplitscreen() { return m_drcOutputEnabled; }
void CConsoleMinecraftApp::setIgnoreMainMenuInputDuringInvite(bool value) { m_ignoreMainMenuInputDuringInvite = value; }
void CConsoleMinecraftApp::LoadLocalDLCImages() { m_dlcInfo->LoadLocalDLCImages(); }
void CConsoleMinecraftApp::FreeLocalDLCImages() { m_dlcInfo->FreeLocalDLCImages(); }
void CConsoleMinecraftApp::ReadBannedList(int, eTMSAction, bool) { }
