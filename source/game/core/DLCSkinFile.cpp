#include "game/core/DLCSkinFile.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ChinaDlcFilter.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/data/StringTable.hpp"

extern CMinecraftApp g_MinecraftApp;

DLCSkinFile::DLCSkinFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Skin, path), mSkinAdjustments(), mFree(false), mAdditionalBoxes()
{
    mHead = L"buckets_";
    mBody = L"buckets_";
    mArm = L"buckets_";
    mLeg = L"buckets_";
    mCape = L"buckets_";
    mElytra = L"buckets_";
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
    g_MinecraftApp.AddMemoryTextureFile((int*)&m_path, (int*)data, size);
}

void DLCSkinFile::addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value)
{
    switch (parameter)
    {
    case (DLCManager::EDLCParameterType)0: mHead = value; break;
    case (DLCManager::EDLCParameterType)1: mBody = value; break;
    case (DLCManager::EDLCParameterType)2: mArm = value; break;
    case (DLCManager::EDLCParameterType)3: mFree = true; break;
    case (DLCManager::EDLCParameterType)5: mLeg = value; break;
    default: break;
    }
}

std::wstring DLCSkinFile::getParameterAsString(DLCManager::EDLCParameterType, const std::wstring& name)
{
    return lookupString(name, L"");
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
    return &mSkinAdjustments;
}

bool DLCSkinFile::isFree()
{
    return mFree;
}
