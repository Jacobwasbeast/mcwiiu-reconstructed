#include "platform/wiiu/cNintendoDLCInfo.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    void* hEvent;
};

int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode,
               void* securityAttributes, unsigned int creationDisposition,
               unsigned int flagsAndAttributes, int templateFile);
unsigned int GetFileSize(int file, unsigned int* fileSizeHigh);
int ReadFile(int file, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead,
             _OVERLAPPED* overlapped);
int CloseHandle(int handle);

cNintendoDLCInfo::cNintendoDLCInfo()
{
}

unsigned int cNintendoDLCInfo::GetDLCInfoCount()
{
    return (unsigned int)m_dlcInfoByCatalogIndex.size();
}

void cNintendoDLCInfo::AddDLCInfo(DLC_INFO* pDLCInfo)
{
    m_dlcInfoByCatalogIndex[pDLCInfo->CatalogIndex] = pDLCInfo;

    wchar_t packName[64];
    mbstowcs(packName, pDLCInfo->PackName, 64);
    packName[63] = L'\0';
    m_dlcInfoByPackName[std::wstring(packName)] = pDLCInfo;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoForPackID(unsigned int packID)
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        if (it->second->PackID == packID)
        {
            return it->second;
        }
    }

    return NULL;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoFromBanner(char* pBannerName)
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        if (std::strcmp(it->second->BannerName, pBannerName) == 0)
        {
            return it->second;
        }
    }

    return NULL;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoByCatalogIndex(unsigned int catalogIndex)
{
    boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.find(catalogIndex);
    if (it != m_dlcInfoByCatalogIndex.end())
    {
        return it->second;
    }

    return NULL;
}

bool cNintendoDLCInfo::GetDLCPurchased(char* pPackName)
{
    wchar_t packName[64];
    mbstowcs(packName, pPackName, 64);
    packName[63] = L'\0';

    boost::unordered_map<std::wstring, DLC_INFO*>::iterator it = m_dlcInfoByPackName.find(std::wstring(packName));
    if (it != m_dlcInfoByPackName.end())
    {
        return it->second->Purchased;
    }

    return false;
}

bool cNintendoDLCInfo::SetDLCPurchased(unsigned int catalogIndex, bool purchased)
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        if (it->second->CatalogIndex == catalogIndex)
        {
            it->second->Purchased = purchased;
            return true;
        }
    }

    return false;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoByIndex(unsigned int index)
{
    unsigned int currentIndex = 0;
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        if (currentIndex == index)
        {
            return it->second;
        }
        ++currentIndex;
    }

    return NULL;
}

DLC_INFO* cNintendoDLCInfo::GetDLCInfoByRomName(const char* pRomName)
{
    std::string romName(pRomName);
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (romName.compare(0, romName.size(), pDLCInfo->RomName, std::strlen(pDLCInfo->RomName)) == 0 &&
            std::strlen(pDLCInfo->RomName) == romName.size())
        {
            return pDLCInfo;
        }
    }

    return NULL;
}

void cNintendoDLCInfo::LoadLocalDLCImages()
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        LoadLocalDLCImage(it->second);
    }
}

void cNintendoDLCInfo::FreeLocalDLCImages()
{
    for (boost::unordered_map<unsigned int, DLC_INFO*>::iterator it = m_dlcInfoByCatalogIndex.begin();
         it != m_dlcInfoByCatalogIndex.end(); ++it)
    {
        DLC_INFO* pDLCInfo = it->second;
        if (pDLCInfo->ImageSize != 0)
        {
            std::free(pDLCInfo->pImageData);
            pDLCInfo->pImageData = NULL;
            pDLCInfo->ImageSize = 0;
        }
    }
}

void cNintendoDLCInfo::LoadLocalDLCImage(DLC_INFO* pDLCInfo)
{
    char fileName[68];
    std::sprintf(fileName, "WiiU/DLCImages/%s.png", pDLCInfo->BannerName);

    int file = CreateFileA(fileName, 0x80000000, 0, NULL, 3, 0x80, 0);
    if (file != -1)
    {
        unsigned int fileSizeHigh = 0;
        unsigned int fileSize = GetFileSize(file, &fileSizeHigh);
        pDLCInfo->ImageSize = fileSize;
        if (fileSize != 0)
        {
            pDLCInfo->pImageData = std::malloc(fileSize);
            unsigned int bytesRead = 0;
            if (ReadFile(file, pDLCInfo->pImageData, pDLCInfo->ImageSize, &bytesRead, NULL) == 0)
            {
                std::free(pDLCInfo->pImageData);
                pDLCInfo->ImageSize = 0;
            }
        }
        CloseHandle(file);
    }
}
