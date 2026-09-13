#include "game/core/DLCManager.hpp"

#include <sstream>
#include <wchar.h>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/client/render/TexturePackRepository.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;
std::vector<std::wstring> g_dlcSkinNames;

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    void* hEvent;
};

std::wstring char16towstring(wchar_t* pText);
std::wstring convStringToWstring(const std::string& text);
char* wstringtofilename(const std::wstring& text, char (&fileName)[256]);

int CreateFileA(const char* pFileName, unsigned int desiredAccess, unsigned int shareMode, void* pSecurityAttributes, unsigned int creationDisposition, unsigned int flagsAndAttributes, int hTemplateFile);
unsigned int GetFileSize(int hFile, unsigned int* pFileSizeHigh);
unsigned int ReadFile(int hFile, void* pBuffer, unsigned int bytesToRead, unsigned int* pBytesRead, _OVERLAPPED* pOverlapped);
void CloseHandle(int hFile);
unsigned int GetLastError();

static const wchar_t* s_parameterNames[] =
{
    L"DISPLAYNAME",
    L"THEMENAME",
    L"LOC_KEY",
    L"FREE",
    L"CREDIT",
    L"CAPEPATH",
    L"BOX",
    L"ANIM",
    L"GAME_FLAGS",
    L"PACKID",
    L"NETHERPARTICLECOLOUR",
    L"ENCHANTTEXTCOLOUR",
    L"ENCHANTTEXTFOCUSCOLOUR",
    L"DATAPATH",
    L"PACKVERSION",
    L"DISPLAYNAMEID",
    L"THEMENAMEID",
    L"CLOUDHEIGHT",
    L"CREDITID",
    L"DISABLEMIPMAP",
    L"OFFSET",
    L"XMLVERSION"
};

static char s_dlcDataFileName[256];

DLCManager::DLCManager()
{
    m_bCorruptDLCFound = true;
}

DLCManager::~DLCManager()
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        delete *it;
    }
}

DLCManager::EDLCParameterType DLCManager::getParameterType(const std::wstring& name)
{
    for (int i = 0; i < eDLCParameterType_NumTypes; i++)
    {
        if (name == s_parameterNames[i])
        {
            return (EDLCParameterType)i;
        }
    }

    return eDLCParameterType_Invalid;
}

int DLCManager::getPackCount(EDLCType type)
{
    if (type == eDLCType_All)
    {
        return (int)m_packs.size();
    }

    int count = 0;

    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        if ((*it)->getDLCItemsCount(type) != 0)
        {
            count++;
        }
    }

    return count;
}

void DLCManager::addPack(DLCPack* pPack)
{
    m_packs.push_back(pPack);
}

void DLCManager::removePack(DLCPack* pPack)
{
    if (pPack != NULL)
    {
        std::vector<DLCPack*>::iterator it = m_packs.begin();

        while (it != m_packs.end() && *it != pPack)
        {
            ++it;
        }

        if (it != m_packs.end())
        {
            m_packs.erase(it);
        }

        delete pPack;
    }
}

void DLCManager::LanguageChanged()
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        (*it)->UpdateLanguage();
    }
}

DLCPack* DLCManager::getPack(const std::wstring& name)
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;

        if (pPack->getName() == name)
        {
            return pPack;
        }
    }

    return NULL;
}

DLCPack* DLCManager::getPackFromPackID(int packID)
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;

        if ((int)pPack->GetPackId() == packID)
        {
            return pPack;
        }
    }

    return NULL;
}

DLCPack* DLCManager::getPack(unsigned int index, EDLCType type)
{
    if (type == eDLCType_All)
    {
        if (index < m_packs.size())
        {
            return m_packs[index];
        }
    }
    else
    {
        unsigned int matchIndex = 0;

        for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
        {
            DLCPack* pPack = *it;

            if (pPack->getDLCItemsCount(type) != 0)
            {
                if (matchIndex == index)
                {
                    return pPack;
                }

                matchIndex++;
            }
        }
    }

    return NULL;
}

DLCFile* DLCManager::getSkinFile(const std::wstring& fileName)
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCFile* pFile = (*it)->getSkinFile(fileName);

        if (pFile != NULL)
        {
            return pFile;
        }
    }

    return NULL;
}

DLCPack* DLCManager::getPackContainingSkin(const std::wstring& fileName)
{
    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;

        if (pPack->getDLCItemsCount(eDLCType_Skin) != 0 && pPack->doesPackContainSkin(fileName))
        {
            return pPack;
        }
    }

    return NULL;
}

bool DLCManager::IsDlcPackTemporarilyFree(int packID)
{
    bool bFree = false;
    long long now = System::currentUnixEpochTimeSeconds();

    boost::unordered_map<int, FreePackPeriod>::iterator it = m_freeDLCPackPeriods.find(packID);

    if (it != m_freeDLCPackPeriods.end() && it->second.startTime <= now && now <= it->second.endTime)
    {
        bFree = true;
    }

    return bFree;
}

DLCFile* DLCManager::getSkinFile(int skinIndex, bool& bIsDefaultSkin, bool& bHasPurchasedPack)
{
    DLCFile* pFile = NULL;

    bIsDefaultSkin = false;
    bHasPurchasedPack = false;

    if (skinIndex < 0)
    {
        wchar_t fileName[256];

        swprintf(fileName, 256, L"dlcskin%08d.png", skinIndex & 0x7FFFFFFF);

        DLCPack* pPack = getPackContainingSkin(fileName);

        if (pPack != NULL)
        {
            pFile = pPack->getSkinFile(fileName);
            bHasPurchasedPack = pPack->hasPurchasedPack();
        }
    }
    else if ((skinIndex & 0x7FFFFFE0) == 0)
    {
        bIsDefaultSkin = true;
        bHasPurchasedPack = true;
    }

    return pFile;
}

int DLCManager::checkForCorruptDLCAndAlert(bool bShowMessage)
{
    int numCorruptPacks = m_numCorruptPacks;
    DLCPack* pFirstCorruptPack = NULL;

    for (std::vector<DLCPack*>::iterator it = m_packs.begin(); it != m_packs.end(); ++it)
    {
        DLCPack* pPack = *it;

        if (pPack->IsCorrupt())
        {
            numCorruptPacks++;

            if (pFirstCorruptPack == NULL)
            {
                pFirstCorruptPack = pPack;
            }
        }
    }

    if (numCorruptPacks != 0 && bShowMessage)
    {
        int messageParameters[1];

        messageParameters[0] = 0xB9A1BF4D;

        if (numCorruptPacks == 1 && pFirstCorruptPack != NULL)
        {
            wchar_t messageText[132];

            swprintf(messageText, 132, L"%ls\n\n%%ls", pFirstCorruptPack->getName().c_str());

            gConsoleUIController.RequestErrorMessage(0xE3D8522A, 0xA69E5B0D, messageParameters, 1, g_pApp->GetPrimaryUserIndex(), NULL, NULL, messageText, true);
        }
        else
        {
            gConsoleUIController.RequestErrorMessage(0xE3D8522A, 0x75E79C02, messageParameters, 1, g_pApp->GetPrimaryUserIndex(), NULL, NULL, NULL, true);
        }
    }

    m_bCorruptDLCFound = false;

    return numCorruptPacks;
}

bool DLCManager::processDLCDataFile(unsigned int& numItemsAdded, unsigned char* pData, unsigned int size, DLCPack* pPack, bool bTakeDataOwnership)
{
    boost::unordered_map<int, EDLCParameterType> parameterTypes;
    bool bHasXMLVersion = false;

    if (*(unsigned int*)pData < 3)
    {
        delete[] pData;
        return false;
    }

    pPack->SetDataPointer(pData, bTakeDataOwnership);

    unsigned int parameterCount = ((unsigned int*)pData)[1];
    unsigned int offset = 8;
    unsigned int xmlVersion = 0;

    if (parameterCount != 0)
    {
        for (unsigned int i = 0; i < parameterCount; i++)
        {
            unsigned int* pParameter = (unsigned int*)(pData + offset);
            EDLCParameterType parameterType = getParameterType(char16towstring((wchar_t*)(pParameter + 2)));

            if (parameterType != eDLCParameterType_Invalid)
            {
                parameterTypes[(int)pParameter[0]] = parameterType;
            }

            offset += pParameter[1] * 2 + 12;

            if (parameterType == eDLCParameterType_XMLVersion)
            {
                bHasXMLVersion = true;
            }
        }

        if (bHasXMLVersion)
        {
            xmlVersion = *(unsigned int*)(pData + offset);
            offset += 4;
        }
    }

    unsigned int itemCount = *(unsigned int*)(pData + offset);
    offset += 4;

    unsigned int dataOffset = offset;

    for (unsigned int i = 0; i < itemCount; i++)
    {
        dataOffset += ((unsigned int*)(pData + dataOffset))[2] * 2 + 16;
    }

    unsigned int* pItemData = (unsigned int*)(pData + dataOffset);

    while (itemCount != 0)
    {
        unsigned int* pItemHeader = (unsigned int*)(pData + offset);
        EDLCType itemType = (EDLCType)pItemHeader[1];

        DLCFile* pFile = NULL;
        DLCPack* pTexturePack = NULL;
        DLCPack* pChildPack = NULL;

        if (itemType == eDLCType_TexturePack)
        {
            pTexturePack = new DLCPack(pPack->getName(), pPack->getLicenseMask());
        }
        else if (itemType == eDLCType_SkinPack)
        {
            pChildPack = new DLCPack(pPack->getName(), pPack->getLicenseMask());
        }
        else if (itemType != eDLCType_Pack)
        {
            pFile = pPack->addFile(itemType, char16towstring((wchar_t*)(pItemHeader + 3)));

            if (itemType == eDLCType_Skin && bHasXMLVersion)
            {
                wchar_t versionText[64];

                swprintf(versionText, 64, L"%i", xmlVersion);

                pFile->addParameter(eDLCParameterType_XMLVersion, versionText);
            }
        }

        unsigned int itemParameterCount = pItemData[0];
        pItemData++;

        for (unsigned int i = 0; i < itemParameterCount; i++)
        {
            boost::unordered_map<int, EDLCParameterType>::iterator it = parameterTypes.find((int)pItemData[0]);

            if (it != parameterTypes.end())
            {
                if (itemType == eDLCType_Pack)
                {
                    pPack->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                    numItemsAdded++;
                }
                else if (pTexturePack != NULL)
                {
                    pTexturePack->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                }
                else if (pChildPack != NULL)
                {
                    pChildPack->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                }
                else if (pFile != NULL)
                {
                    pFile->addParameter(it->second, char16towstring((wchar_t*)(pItemData + 2)));
                }
            }

            pItemData = (unsigned int*)((unsigned char*)pItemData + pItemData[1] * 2 + 12);
        }

        if (pTexturePack != NULL)
        {
            unsigned int numChildItemsAdded = 0;
            bool bLoaded = processDLCDataFile(numChildItemsAdded, (unsigned char*)pItemData, pItemHeader[0], pTexturePack, false);

            pPack->SetDataPointer(NULL, false);

            if (!bLoaded || numChildItemsAdded == 0)
            {
                delete pTexturePack;
                numItemsAdded++;
            }
            else
            {
                pPack->addChildPack(pTexturePack);

                if (pTexturePack->getDLCItemsCount(eDLCType_Texture) != 0)
                {
                    unsigned int packID = pTexturePack->GetPackId();

                    Minecraft::GetInstance()->mTexturePackRepository->addTexturePackFromDLC(pTexturePack, packID);
                }

                numItemsAdded++;
            }
        }
        else if (pChildPack != NULL)
        {
            unsigned int numChildItemsAdded = 0;
            bool bLoaded = processDLCDataFile(numChildItemsAdded, (unsigned char*)pItemData, pItemHeader[0], pChildPack, false);

            pPack->SetDataPointer(NULL, false);

            if (!bLoaded || numChildItemsAdded == 0)
            {
                delete pChildPack;
                numItemsAdded++;
            }
            else
            {
                pPack->addChildPack(pChildPack);
                numItemsAdded++;
            }
        }
        else if (pFile != NULL)
        {
            pFile->addData((unsigned char*)pItemData, pItemHeader[0]);

            if (itemType == eDLCType_Skin)
            {
                g_dlcSkinNames.push_back(char16towstring((wchar_t*)(pItemHeader + 3)));
            }

            numItemsAdded++;
        }

        itemCount--;
        offset += pItemHeader[2] * 2 + 16;
        pItemData = (unsigned int*)((unsigned char*)pItemData + pItemHeader[0]);
    }

    if (pPack->getDLCItemsCount(eDLCType_GameRules) != 0 || pPack->getDLCItemsCount(eDLCType_GameRulesHeader) != 0)
    {
        gApp.mGameRuleManager.loadGameRules(pPack);
    }

    pPack->getDLCItemsCount(eDLCType_Audio);

    return true;
}

bool DLCManager::readDLCDataFile(unsigned int& numItemsAdded, const std::string& fileName, DLCPack* pPack, unsigned int packID, bool bTakeDataOwnership)
{
    std::wstring wideFileName = convStringToWstring(fileName);

    int hFile = CreateFileA(fileName.c_str(), 0x80000000, 0, NULL, 3, 0x80, 0);

    if (hFile == -1)
    {
        GetLastError();

        if (numItemsAdded == 0)
        {
            removePack(pPack);
        }

        return false;
    }

    unsigned int fileSize = GetFileSize(hFile, NULL);
    unsigned char* pData = new unsigned char[fileSize];
    unsigned int bytesRead = 0;

    if (ReadFile(hFile, pData, fileSize, &bytesRead, NULL) == 0)
    {
        pPack->SetIsCorrupt(true);
        m_bCorruptDLCFound = true;

        return false;
    }

    CloseHandle(hFile);

    return processDLCDataFile(numItemsAdded, pData, bytesRead, pPack, bTakeDataOwnership);
}

bool DLCManager::readDLCDataFile(unsigned int& numItemsAdded, const std::wstring& fileName, DLCPack* pPack, unsigned int packID, bool bTakeDataOwnership)
{
    std::string narrowFileName = wstringtofilename(fileName, s_dlcDataFileName);

    return readDLCDataFile(numItemsAdded, narrowFileName, pPack, packID, bTakeDataOwnership);
}

unsigned int DLCManager::retrievePackID(unsigned char* pData, unsigned int size)
{
    unsigned int packID = 0;
    std::wstring packName = L"";
    boost::unordered_map<int, EDLCParameterType> parameterTypes;
    bool bHasXMLVersion = false;

    if (*(unsigned int*)pData < 3)
    {
        return 0;
    }

    unsigned int parameterCount = ((unsigned int*)pData)[1];
    unsigned int offset = 8;

    if (parameterCount != 0)
    {
        for (unsigned int i = 0; i < parameterCount; i++)
        {
            unsigned int* pParameter = (unsigned int*)(pData + offset);
            EDLCParameterType parameterType = getParameterType(char16towstring((wchar_t*)(pParameter + 2)));

            if (parameterType != eDLCParameterType_Invalid)
            {
                parameterTypes[(int)pParameter[0]] = parameterType;
            }

            offset += pParameter[1] * 2 + 12;

            if (parameterType == eDLCParameterType_XMLVersion)
            {
                bHasXMLVersion = true;
            }
        }

        if (bHasXMLVersion)
        {
            offset += 4;
        }
    }

    unsigned int itemCount = *(unsigned int*)(pData + offset);
    offset += 4;

    unsigned int dataOffset = offset;

    for (unsigned int i = 0; i < itemCount; i++)
    {
        dataOffset += ((unsigned int*)(pData + dataOffset))[2] * 2 + 16;
    }

    unsigned int* pItemData = (unsigned int*)(pData + dataOffset);

    while (itemCount != 0)
    {
        unsigned int* pItemHeader = (unsigned int*)(pData + offset);
        unsigned int itemParameterCount = pItemData[0];

        pItemData++;

        EDLCType itemType = (EDLCType)pItemHeader[1];

        for (unsigned int i = 0; i < itemParameterCount; i++)
        {
            boost::unordered_map<int, EDLCParameterType>::iterator it = parameterTypes.find((int)pItemData[0]);

            if (it != parameterTypes.end())
            {
                if (itemType == eDLCType_UIData)
                {
                    if (it->second == eDLCParameterType_PackID)
                    {
                        std::wstring packIDText = char16towstring((wchar_t*)(pItemData + 2));
                        std::wstringstream stream;

                        stream << std::dec << packIDText.c_str();
                        stream >> packID;

                        parameterTypes.clear();

                        return packID;
                    }
                }
                else if (itemType == eDLCType_Skin && packName.empty() && it->second == eDLCParameterType_DisplayName)
                {
                    packName = char16towstring((wchar_t*)(pItemData + 2));
                }
            }

            pItemData = (unsigned int*)((unsigned char*)pItemData + pItemData[1] * 2 + 12);
        }

        itemCount--;
        offset += pItemHeader[2] * 2 + 16;
        pItemData = (unsigned int*)((unsigned char*)pItemData + pItemHeader[0]);
    }

    parameterTypes.clear();

    return packID;
}

unsigned int DLCManager::retrievePackIDFromDLCDataFile(const std::string& fileName)
{
    std::wstring wideFileName = convStringToWstring(fileName);

    int hFile = CreateFileA(fileName.c_str(), 0x80000000, 0, NULL, 3, 0x80, 0);

    if (hFile == -1)
    {
        return 0;
    }

    unsigned int fileSize = GetFileSize(hFile, NULL);
    unsigned char* pData = new unsigned char[fileSize];
    unsigned int bytesRead = 0;

    if (ReadFile(hFile, pData, fileSize, &bytesRead, NULL) == 0)
    {
        delete[] pData;

        return 0;
    }

    CloseHandle(hFile);

    unsigned int packID = retrievePackID(pData, bytesRead);

    delete[] pData;

    return packID;
}

bool DLCManager::IsMiniGameTemporarilyFree(int miniGameID)
{
    bool bFree = false;
    long long now = System::currentUnixEpochTimeSeconds();

    boost::unordered_map<int, FreePackPeriod>::iterator it = m_freeMiniGamePeriods.find(miniGameID);

    if (it != m_freeMiniGamePeriods.end() && it->second.startTime <= now && now <= it->second.endTime)
    {
        bFree = true;
    }

    return bFree;
}
