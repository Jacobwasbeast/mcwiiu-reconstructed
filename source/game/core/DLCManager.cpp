#include "game/core/DLCManager.hpp"
#include "game/core/StringIds.hpp"

#include <sstream>
#include <fstream>
#include <cstring>
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
#include "platform/wiiu/CProfile.hpp"
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;
std::vector<std::wstring> g_dlcSkinNames;

char* wstringtofilename(const std::wstring& text, char (&fileName)[256]);
struct _OVERLAPPED;
int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode,
                void* securityAttributes, unsigned int creationDisposition,
                unsigned int flagsAndAttributes, int templateFile);
unsigned int GetFileSize(int file, unsigned int* highSize);
unsigned int ReadFile(int file, void* buffer, unsigned int bytesToRead,
                      unsigned int* bytesRead, _OVERLAPPED* overlapped);

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

class DLCDataReader
{
public:
    DLCDataReader(unsigned char* data, unsigned int size) : mData(data), mSize(size) {}

    void read32(unsigned int offset, unsigned int& value) const
    {
        std::memcpy(&value, mData + offset, sizeof(value));
    }

    void read16(unsigned int offset, unsigned short& value) const
    {
        std::memcpy(&value, mData + offset, sizeof(value));
    }

    void readString(unsigned int offset, unsigned int characterCount, std::wstring& value) const
    {
        value.clear();
        value.reserve(characterCount);
        for (unsigned int i = 0; i < characterCount; ++i)
        {
            unsigned short character = 0;
            read16(offset + i * 2, character);
            value.push_back(static_cast<wchar_t>(character));
        }
    }

    unsigned char* bytes(unsigned int offset, unsigned int length) const
    {
        return mData + offset;
    }

    unsigned char* mData;
    unsigned int mSize;
};

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

        messageParameters[0] = IDS_CONFIRM_OK;

        if (numCorruptPacks == 1 && pFirstCorruptPack != NULL)
        {
            wchar_t messageText[132];

            swprintf(messageText, 132, L"%ls\n\n%%ls", pFirstCorruptPack->getName().c_str());

            gConsoleUIController.RequestErrorMessage(IDS_CORRUPT_DLC_TITLE, IDS_CORRUPT_DLC, messageParameters, 1, gProfile->GetPrimaryPad(), NULL, NULL, messageText, true);
        }
        else
        {
            gConsoleUIController.RequestErrorMessage(IDS_CORRUPT_DLC_TITLE, 0x75E79C02, messageParameters, 1, gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        }
    }

    m_bCorruptDLCFound = false;

    return numCorruptPacks;
}

bool DLCManager::processDLCDataFile(unsigned int& numItemsAdded, unsigned char* pData, unsigned int size, DLCPack* pPack, bool bTakeDataOwnership)
{
    DLCDataReader reader(pData, size);
    boost::unordered_map<int, EDLCParameterType> parameterTypes;
    bool bHasXMLVersion = false;

    unsigned int version = 0;
    reader.read32(0, version);
    if (version < 3)
    {
        delete[] pData;
        return false;
    }

    pPack->SetDataPointer(pData, bTakeDataOwnership);

    unsigned int parameterCount = 0;
    reader.read32(4, parameterCount);
    unsigned int offset = 8;
    unsigned int xmlVersion = 0;

    if (parameterCount != 0)
    {
        for (unsigned int i = 0; i < parameterCount; i++)
        {
            unsigned int parameterId = 0;
            unsigned int parameterLength = 0;
            reader.read32(offset, parameterId);
            reader.read32(offset + 4, parameterLength);
            std::wstring parameterName;
            reader.readString(offset + 8, parameterLength, parameterName);
            EDLCParameterType parameterType = getParameterType(parameterName);

            if (parameterType != eDLCParameterType_Invalid)
            {
                parameterTypes[(int)parameterId] = parameterType;
            }

            offset += parameterLength * 2 + 12;

            if (parameterType == eDLCParameterType_XMLVersion)
            {
                bHasXMLVersion = true;
            }
        }

        if (bHasXMLVersion)
        {
            reader.read32(offset, xmlVersion);
            offset += 4;
        }
    }

    unsigned int itemCount = 0;
    reader.read32(offset, itemCount);
    offset += 4;

    unsigned int dataOffset = offset;

    for (unsigned int i = 0; i < itemCount; i++)
    {
        unsigned int itemNameLength = 0;
        reader.read32(dataOffset + 8, itemNameLength);
        dataOffset += itemNameLength * 2 + 16;
    }

    while (itemCount != 0)
    {
        unsigned int itemLength = 0;
        unsigned int itemTypeValue = 0;
        unsigned int itemNameLength = 0;
        reader.read32(offset, itemLength);
        reader.read32(offset + 4, itemTypeValue);
        reader.read32(offset + 8, itemNameLength);
        EDLCType itemType = (EDLCType)itemTypeValue;

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
            std::wstring itemName;
            reader.readString(offset + 12, itemNameLength, itemName);
            pFile = pPack->addFile(itemType, itemName);

            if (itemType == eDLCType_Skin && bHasXMLVersion)
            {
                wchar_t versionText[64];

                swprintf(versionText, 64, L"%i", xmlVersion);

                pFile->addParameter(eDLCParameterType_XMLVersion, versionText);
            }
        }

        unsigned int itemParameterCount = 0;
        reader.read32(dataOffset, itemParameterCount);
        unsigned int parameterOffset = dataOffset + 4;

        for (unsigned int i = 0; i < itemParameterCount; i++)
        {
            unsigned int parameterId = 0;
            unsigned int parameterLength = 0;
            reader.read32(parameterOffset, parameterId);
            reader.read32(parameterOffset + 4, parameterLength);
            boost::unordered_map<int, EDLCParameterType>::iterator it = parameterTypes.find((int)parameterId);

            if (it != parameterTypes.end())
            {
                if (itemType == eDLCType_Pack)
                {
                    std::wstring parameterValue;
                    reader.readString(parameterOffset + 8, parameterLength, parameterValue);
                    pPack->addParameter(it->second, parameterValue);
                    numItemsAdded++;
                }
                else if (pTexturePack != NULL)
                {
                    std::wstring parameterValue;
                    reader.readString(parameterOffset + 8, parameterLength, parameterValue);
                    pTexturePack->addParameter(it->second, parameterValue);
                }
                else if (pChildPack != NULL)
                {
                    std::wstring parameterValue;
                    reader.readString(parameterOffset + 8, parameterLength, parameterValue);
                    pChildPack->addParameter(it->second, parameterValue);
                }
                else if (pFile != NULL)
                {
                    std::wstring parameterValue;
                    reader.readString(parameterOffset + 8, parameterLength, parameterValue);
                    pFile->addParameter(it->second, parameterValue);
                }
            }

            parameterOffset += parameterLength * 2 + 12;
        }

        if (pTexturePack != NULL)
        {
            unsigned int numChildItemsAdded = 0;
            unsigned char* childData = reader.bytes(parameterOffset, itemLength);
            bool bLoaded = processDLCDataFile(numChildItemsAdded, childData, itemLength, pTexturePack, false);

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
            unsigned char* childData = reader.bytes(parameterOffset, itemLength);
            bool bLoaded = processDLCDataFile(numChildItemsAdded, childData, itemLength, pChildPack, false);

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
            unsigned char* fileData = reader.bytes(parameterOffset, itemLength);
            pFile->addData(fileData, itemLength);

            if (itemType == eDLCType_Skin)
            {
                std::wstring skinName;
                reader.readString(offset + 12, itemNameLength, skinName);
                g_dlcSkinNames.push_back(skinName);
            }

            numItemsAdded++;
        }

        itemCount--;
        offset += itemNameLength * 2 + 16;
        dataOffset = parameterOffset + itemLength;
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
    int file = CreateFileA(fileName.c_str(), 0x80000000, 0, NULL, 3, 0x80, 0);
    if (file == -1)
    {
        if (numItemsAdded == 0)
        {
            removePack(pPack);
        }

        return false;
    }

    const unsigned int fileSize = GetFileSize(file, NULL);
    unsigned char* pData = new unsigned char[fileSize];
    unsigned int bytesRead = 0;
    if (ReadFile(file, pData, fileSize, &bytesRead, NULL) == 0)
    {
        pPack->SetIsCorrupt(true);
        m_bCorruptDLCFound = true;
        CloseHandle(file);
        delete[] pData;
        return false;
    }
    CloseHandle(file);

    return processDLCDataFile(numItemsAdded, pData, bytesRead, pPack, bTakeDataOwnership);
}

bool DLCManager::readDLCDataFile(unsigned int& numItemsAdded, const std::wstring& fileName, DLCPack* pPack, unsigned int packID, bool bTakeDataOwnership)
{
    std::string narrowFileName = wstringtofilename(fileName, s_dlcDataFileName);

    return readDLCDataFile(numItemsAdded, narrowFileName, pPack, packID, bTakeDataOwnership);
}

unsigned int DLCManager::retrievePackID(unsigned char* pData, unsigned int size)
{
    DLCDataReader reader(pData, size);
    unsigned int packID = 0;
    std::wstring packName = L"";
    boost::unordered_map<int, EDLCParameterType> parameterTypes;
    bool bHasXMLVersion = false;

    unsigned int version = 0;
    reader.read32(0, version);
    if (version < 3)
    {
        return 0;
    }

    unsigned int parameterCount = 0;
    reader.read32(4, parameterCount);
    unsigned int offset = 8;

    if (parameterCount != 0)
    {
        for (unsigned int i = 0; i < parameterCount; i++)
        {
            unsigned int nameLength = 0;
            unsigned int parameterId = 0;
            reader.read32(offset, parameterId);
            reader.read32(offset + 4, nameLength);
            std::wstring parameterName;
            reader.readString(offset + 8, nameLength, parameterName);
            EDLCParameterType parameterType = getParameterType(parameterName);

            if (parameterType != eDLCParameterType_Invalid)
            {
                parameterTypes[(int)parameterId] = parameterType;
            }

            offset += nameLength * 2 + 12;

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

    unsigned int itemCount = 0;
    reader.read32(offset, itemCount);
    offset += 4;

    unsigned int dataOffset = offset;

    for (unsigned int i = 0; i < itemCount; i++)
    {
        unsigned int itemLength = 0;
        reader.read32(dataOffset + 8, itemLength);
        dataOffset += itemLength * 2 + 16;
    }

    while (itemCount != 0)
    {
        unsigned int itemLength = 0;
        unsigned int itemTypeValue = 0;
        unsigned int itemParameterCount = 0;
        reader.read32(offset, itemLength);
        reader.read32(offset + 4, itemTypeValue);
        reader.read32(dataOffset, itemParameterCount);
        const unsigned int itemType = itemTypeValue;
        unsigned int parameterOffset = dataOffset + 4;

        for (unsigned int i = 0; i < itemParameterCount; i++)
        {
            unsigned int parameterId = 0;
            unsigned int valueLength = 0;
            reader.read32(parameterOffset, parameterId);
            reader.read32(parameterOffset + 4, valueLength);
            boost::unordered_map<int, EDLCParameterType>::iterator it = parameterTypes.find((int)parameterId);

            if (it != parameterTypes.end())
            {
                if (itemType == eDLCType_UIData)
                {
                    if (it->second == eDLCParameterType_PackID)
                    {
                        std::wstring packIDText;
                        reader.readString(parameterOffset + 8, valueLength, packIDText);
                        std::wstringstream stream;

                        stream << std::dec << packIDText.c_str();
                        stream >> packID;

                        parameterTypes.clear();

                        return packID;
                    }
                }
                else if (itemType == eDLCType_Skin && packName.empty() && it->second == eDLCParameterType_DisplayName)
                {
                    reader.readString(parameterOffset + 8, valueLength, packName);
                }
            }

            parameterOffset += valueLength * 2 + 12;
        }

        itemCount--;
        offset += itemLength * 2 + 16;
        dataOffset += itemLength;
    }

    parameterTypes.clear();

    return packID;
}

unsigned int DLCManager::retrievePackIDFromDLCDataFile(const std::string& fileName)
{
    std::wstring wideFileName = convStringToWstring(fileName);

    std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        return 0;
    }

    file.seekg(0, std::ios::end);
    const std::streamoff end = file.tellg();
    if (end < 0)
        return 0;
    const unsigned int fileSize = static_cast<unsigned int>(end);
    file.seekg(0, std::ios::beg);
    unsigned char* pData = new unsigned char[fileSize];
    file.read(reinterpret_cast<char*>(pData), fileSize);
    const unsigned int bytesRead = static_cast<unsigned int>(file.gcount());
    if (!file && !file.eof())
    {
        delete[] pData;

        return 0;
    }

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
