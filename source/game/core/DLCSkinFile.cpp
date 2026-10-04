#include "game/core/DLCSkinFile.hpp"

#include <cstdio>
#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ChinaDlcFilter.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/StringUtils.hpp"
#include "game/data/StringTable.hpp"

extern CConsoleMinecraftApp gApp;

DLCSkinFile::DLCSkinFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Skin, path), mSkinAdjustments(), mAdditionalBoxes()
{
    mDisplayName = L"";
    mDisplayNameID = L"";
    mThemeName = L"";
    mLocKeySet = false;
    mLocKey = L"";
    mThemeNameID = L"";
    mCapePath = L"";
    mFree = false;
}

bool DLCSkinFile::isFiltered()
{
    return ChinaDlcFilter::instance()->isSkinFiltered(m_path);
}

void DLCSkinFile::addData(unsigned char* data, unsigned int size)
{
    gApp.AddMemoryTextureFile(m_path, data, size);
}

void DLCSkinFile::addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value)
{
    switch (parameter)
    {
    case DLCManager::eDLCParameterType_DisplayName:
        mDisplayName = value;
        break;
    case DLCManager::eDLCParameterType_ThemeName:
        mThemeName = value;
        break;
    case DLCManager::eDLCParameterType_LocKey:
        mLocKey = value;
        mLocKeySet = true;
        break;
    case DLCManager::eDLCParameterType_Free:
        mFree = true;
        break;
    case DLCManager::eDLCParameterType_Credit:
        break;
    case DLCManager::eDLCParameterType_CapePath:
        mCapePath = value;
        break;
    case DLCManager::eDLCParameterType_PackID:
        break;
    case DLCManager::eDLCParameterType_NetherParticleColour:
        break;
    case DLCManager::eDLCParameterType_EnchantTextColour:
        break;
    case DLCManager::eDLCParameterType_EnchantTextFocusColour:
        break;
    case DLCManager::eDLCParameterType_DataPath:
        break;
    case DLCManager::eDLCParameterType_PackVersion:
        break;
    case DLCManager::eDLCParameterType_DisplayNameID:
        mDisplayNameID = value;
        break;
    case DLCManager::eDLCParameterType_ThemeNameID:
        mThemeNameID = value;
        break;
    case DLCManager::eDLCParameterType_CloudHeight:
        break;
    case DLCManager::eDLCParameterType_CreditID:
        m_pPack->AddCreditID(value);
        break;
    case DLCManager::eDLCParameterType_DisableMipMap:
        break;
    case DLCManager::eDLCParameterType_XMLVersion:
        {
            int xmlVersion = 0;
            swscanf(value.c_str(), L"%i", &xmlVersion);
            mSkinAdjustments.mXMLVersion = xmlVersion;
        }
        break;
    default:
        break;
    }
}

std::wstring DLCSkinFile::getParameterAsString(DLCManager::EDLCParameterType parameter, const std::wstring&)
{
    switch (parameter)
    {
    case DLCManager::eDLCParameterType_DisplayName:
        if (!mLocKeySet)
            return lookupString(mDisplayNameID, mDisplayName);
        {
            std::vector<std::wstring> parts = stringSplit(lookupString(mLocKey, mDisplayName), L'-');
            return parts[0];
        }
    case DLCManager::eDLCParameterType_ThemeName:
        if (mLocKeySet)
        {
            std::wstring themeKey = lookupString(mLocKey, mThemeName);
            if (themeKey.find(L"-") != std::wstring::npos)
            {
                std::vector<std::wstring> parts = stringSplit(themeKey, L'-');
                return parts.back();
            }
        }
        return lookupString(mThemeNameID, mThemeName);
    case DLCManager::eDLCParameterType_CapePath:
        return mCapePath;
    default:
        return L"";
    }
}

bool DLCSkinFile::getParameterAsBool(DLCManager::EDLCParameterType parameter)
{
    if (parameter != DLCManager::eDLCParameterType_Free)
        return false;
    return mFree;
}

DLCLocalisationFile* DLCSkinFile::getLocalisationFile()
{
    DLCLocalisationFile* localisation = static_cast<DLCLocalisationFile*>(m_pPack->getFile(
        DLCManager::eDLCType_Localisation, L"localisation.loc", true, true));
    if (localisation == NULL)
    {
        localisation = static_cast<DLCLocalisationFile*>(m_pPack->getFile(
            DLCManager::eDLCType_Localisation, L"languages.loc", true, true));
    }
    return localisation;
}

std::wstring DLCSkinFile::lookupString(const std::wstring& key, const std::wstring& fallback)
{
    DLCLocalisationFile* localisation = getLocalisationFile();
    if (!key.empty() && localisation != NULL)
    {
        StringTable* table = localisation->getStringTable();
        if (table != NULL)
            return std::wstring(table->getString(key));
    }
    return fallback;
}

std::vector<SKIN_BOX*>* DLCSkinFile::getAdditionalBoxes()
{
    return &mAdditionalBoxes;
}

int DLCSkinFile::getAdditionalBoxesCount()
{
    return (int)mAdditionalBoxes.size();
}

_SkinAdjustments DLCSkinFile::getSkinAdjustments()
{
    return mSkinAdjustments;
}

bool DLCSkinFile::isFree()
{
    return mFree;
}
