#include "game/core/DLCSkinFile.hpp"

#include <cstdio>
#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ChinaDlcFilter.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/data/StringTable.hpp"

extern CConsoleMinecraftApp gApp;

DLCSkinFile::DLCSkinFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Skin, path), mLocKeySet(false), mSkinAdjustments(), mFree(false), mAdditionalBoxes()
{
    mDisplayName = L"buckets_";
    mThemeName = L"buckets_";
    mLocKey = L"buckets_";
    mDisplayNameID = L"buckets_";
    mThemeNameID = L"buckets_";
    mCapePath = L"buckets_";
}

DLCSkinFile::~DLCSkinFile()
{
    for (std::vector<SKIN_BOX*>::iterator it = mAdditionalBoxes.begin(); it != mAdditionalBoxes.end(); ++it)
        delete *it;
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
        break;
    case DLCManager::eDLCParameterType_ThemeName:
        if (mLocKeySet)
        {
            std::wstring themeKey = lookupString(mLocKey, mThemeName);
            if (themeKey.find(L"-") == std::wstring::npos)
                return lookupString(mThemeNameID, mThemeName);
            break;
        }
        return lookupString(mThemeNameID, mThemeName);
    case DLCManager::eDLCParameterType_CapePath:
        return mCapePath;
    default:
        break;
    }
    return L"";
}

bool DLCSkinFile::getParameterAsBool(DLCManager::EDLCParameterType parameter)
{
    return parameter == (DLCManager::EDLCParameterType)3 && mFree;
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
    if (m_pPack != NULL && localisation != NULL && localisation->getStringTable() != NULL)
    {
        wchar_t* value = localisation->getStringTable()->getString(key);
        if (value != NULL)
            return std::wstring(value);
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

_SkinAdjustments* DLCSkinFile::getSkinAdjustments()
{
    _SkinAdjustments adjustments = mSkinAdjustments;
    return &adjustments;
}

bool DLCSkinFile::isFree()
{
    return mFree;
}
