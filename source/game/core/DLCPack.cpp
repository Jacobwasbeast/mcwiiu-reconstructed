#include "game/core/DLCPack.hpp"

#include <cstdlib>
#include <cwchar>

#include "game/core/DLCLocalisationFile.hpp"
#include "game/data/StringTable.hpp"

DLCPack::DLCPack(const std::wstring& name, unsigned int licenseMask)
{
    m_licenseMask = licenseMask;
    m_name = name;
    m_dataPath = L"";
    m_packId = 0;
    m_packVersion = 0;
    m_dlcMountIndex = 0xffffffffu;
    m_dlcDeviceId = 0;
    m_isCorrupt = false;
    m_data = NULL;
    m_ownsData = false;
    m_purchaseOfferId = 0;
    m_parentPack = NULL;
}

DLCPack::~DLCPack()
{
    unsigned int i;
    for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
    {
        std::vector<DLCFile*>::iterator it;
        for (it = m_files[i].begin(); it != m_files[i].end(); ++it)
            delete *it;
    }
    for (std::vector<DLCPack*>::iterator child = m_childPacks.begin(); child != m_childPacks.end(); ++child)
        delete *child;
    if (m_ownsData)
        delete[] m_data;
}

DLCFile* DLCPack::getFile(DLCManager::EDLCType type, unsigned int index, bool searchParent, bool searchChildren)
{
    if (type == DLCManager::eDLCType_All)
    {
        unsigned int i;
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
        {
            DLCFile* result = getFile(static_cast<DLCManager::EDLCType>(i), index, true, true);
            if (result != NULL) return result;
        }
        return 0;
    }
    if (index < m_files[type].size() && m_files[type][index] != NULL)
        return m_files[type][index];
    if (searchParent && m_parentPack != NULL)
    {
        DLCFile* result = m_parentPack->getFile(type, index, true, false);
        if (result != NULL) return result;
    }
    if (searchChildren)
    {
        for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
        {
            DLCFile* result = (*it)->getFile(type, index, false, true);
            if (result != NULL) return result;
        }
    }
    return 0;
}

int DLCPack::getDLCItemsCount(DLCManager::EDLCType type)
{
    if (type == DLCManager::eDLCType_All)
    {
        int total = 0;
        unsigned int i;
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
            total += getDLCItemsCount(static_cast<DLCManager::EDLCType>(i));
        return total;
    }
    int total = static_cast<int>(m_files[type].size());
    for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
        total += (*it)->getDLCItemsCount(type);
    return total;
}

DLCFile* DLCPack::getFile(DLCManager::EDLCType type, const std::wstring& name, bool searchParent, bool searchChildren)
{
    int i;
    if (type == DLCManager::eDLCType_All)
    {
        for (i = 0; i < DLCManager::eDLCType_NumTypes; ++i)
        {
            DLCFile* result = getFile(static_cast<DLCManager::EDLCType>(i), name, true, true);
            if (result != NULL) return result;
        }
        return 0;
    }
    for (i = 0; i < static_cast<int>(m_files[type].size()); ++i)
    {
        if (m_files[type][i] != NULL && m_files[type][i]->getParameterAsString(DLCManager::eDLCParameterType_DisplayName, L"") == name)
            return m_files[type][i];
    }
    if (searchParent && m_parentPack != NULL)
    {
        DLCFile* result = m_parentPack->getFile(type, name, true, false);
        if (result != NULL) return result;
    }
    if (searchChildren)
        for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
        {
            DLCFile* result = (*it)->getFile(type, name, false, true);
            if (result != NULL) return result;
        }
    return 0;
}

void DLCPack::UpdateLanguage() {}
std::wstring DLCPack::getName() { return m_name; }
unsigned int DLCPack::GetPackId() { return m_packId; }
DLCFile* DLCPack::getSkinFile(const std::wstring& name) { return getFile(DLCManager::eDLCType_Skin, name, true, true); }
bool DLCPack::doesPackContainFile(DLCManager::EDLCType type, const std::wstring& name, bool searchParent, bool searchChildren) { return getFile(type, name, searchParent, searchChildren) != 0; }
bool DLCPack::doesPackContainSkin(const std::wstring& name) { return doesPackContainFile(DLCManager::eDLCType_Skin, name, true, true); }
bool DLCPack::hasPurchasedPack() { return m_licenseMask != 0; }
bool DLCPack::IsCorrupt() { return m_isCorrupt; }
void DLCPack::SetIsCorrupt(bool corrupt) { m_isCorrupt = corrupt; }
void DLCPack::SetDataPointer(unsigned char* data, bool ownsData) { m_data = data; m_ownsData = ownsData; }
unsigned int DLCPack::getLicenseMask() { return m_licenseMask; }
void DLCPack::addFile(DLCManager::EDLCType type, const std::wstring& name) { m_files[type].push_back(new DLCFile(type, name)); }
void DLCPack::SetPackId(unsigned int id) { m_packId = id; }
void DLCPack::SetPackVersion(unsigned int version) { m_packVersion = version; }
void DLCPack::addParameter(DLCManager::EDLCParameterType type, const std::wstring& value) { m_parameters[static_cast<int>(type)] = value; }
void DLCPack::setParentPack(DLCPack* parent) { m_parentPack = parent; }
void DLCPack::addChildPack(DLCPack* child) { if (child != NULL) { child->SetPackId((child->GetPackId() << 24) | m_packId); m_childPacks.push_back(child); child->setParentPack(this); child->m_name = m_name + child->m_name; } }
std::wstring DLCPack::getFullDataPath() { return m_dataPath; }
unsigned int DLCPack::GetPackVersion() { return m_packVersion; }
DLCPack* DLCPack::GetParentPack() { return m_parentPack; }
unsigned int DLCPack::GetParentPackId() { return m_parentPack == NULL ? 0 : m_parentPack->GetPackId(); }
void DLCPack::SetDLCMountIndex(unsigned int index) { m_dlcMountIndex = index; }
unsigned int DLCPack::GetDLCMountIndex() { return m_parentPack == NULL ? m_dlcMountIndex : m_parentPack->GetDLCMountIndex(); }
void DLCPack::SetDLCDeviceID(int id) { m_dlcDeviceId = id; }
int DLCPack::GetDLCDeviceID() { return m_dlcDeviceId; }
std::vector<DLCPack*>& DLCPack::getChildPacks() { return m_childPacks; }
void DLCPack::AddCreditID(const std::wstring& value) { m_creditIds.push_back(value); }
void DLCPack::AddCreditString(const std::wstring& value) { m_creditStrings.push_back(value); }
bool DLCPack::getParameterAsUInt(DLCManager::EDLCParameterType type, unsigned int& value)
{
    boost::unordered_map<int, std::wstring>::const_iterator it = m_parameters.find(static_cast<int>(type));
    if (it == m_parameters.end())
        return false;
    value = static_cast<unsigned int>(std::wcstoul(it->second.c_str(), NULL, 10));
    return true;
}
void DLCPack::updateLicenseMask(unsigned int licenseMask) { m_licenseMask = licenseMask; }
unsigned int DLCPack::getPurchaseOfferId() { return m_purchaseOfferId; }
int DLCPack::getFileIndexAt(DLCManager::EDLCType type, const std::wstring& name, bool& found)
{
    found = false;
    if (type == DLCManager::eDLCType_All)
        return 0;
    int i;
    for (i = 0; i < static_cast<int>(m_files[type].size()); ++i)
    {
        DLCFile* file = m_files[type][i];
        if (file != NULL && file->getParameterAsString(DLCManager::eDLCParameterType_DisplayName, L"") == name)
        {
            found = true;
            return i;
        }
    }
    for (std::vector<DLCPack*>::iterator it = m_childPacks.begin(); it != m_childPacks.end(); ++it)
    {
        int index = (*it)->getFileIndexAt(type, name, found);
        if (found)
            return index;
    }
    return 0;
}
int DLCPack::getSkinCount() { return getDLCItemsCount(DLCManager::eDLCType_Skin); }
int DLCPack::getSkinIndexAt(const std::wstring& name, bool& found) { return getFileIndexAt(DLCManager::eDLCType_Skin, name, found); }
void DLCPack::getSkinFile(unsigned int index) { getFile(DLCManager::eDLCType_Skin, index, true, true); }
std::wstring DLCPack::getLocalisedDisplayName() { return m_name; }
