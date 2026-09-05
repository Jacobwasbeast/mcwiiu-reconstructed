#include "game/client/render/DLCTexturePack.hpp"

#include <string>

#include "game/client/render/DLCModelsFile.hpp"
#include "game/core/ArchiveFile.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCBehavioursFile.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/data/StringTable.hpp"
#include "game/world/block/DLCMaterialFile.hpp"
#include "game/world/block/MaterialTable.hpp"
#include "game/world/generation/StructureManager.hpp"

extern ConsoleUIController g_ConsoleUIController;
extern DLCManager g_DLCManager;

class DLCMaterialFileAccess : public DLCMaterialFile
{
public:
    DLCMaterialFileAccess(const std::wstring& path) : DLCMaterialFile(path) {}
    MaterialTable* getMaterialTable() { return m_pMaterialTable; }
};

DLCTexturePack::DLCTexturePack(unsigned int id, DLCPack* pack, TexturePack* parentPack)
    : AbstractTexturePack(id, NULL, pack->getName(), parentPack)
    , m_dlcPack(pack)
    , m_loadedPack(NULL)
    , m_stringTable(NULL)
    , m_uiLoaded(0)
    , m_loadingData(0)
    , m_hasData(0)
    , m_hasValidData(0)
    , m_materialTableLoaded(1)
    , m_colourTableLoaded(1)
    , m_padding(0)
    , m_archiveFile(NULL)
{
    if (CMinecraftApp::sInstance.getLevelGenerationOptions() != NULL)
        CMinecraftApp::sInstance.getLevelGenerationOptions()->setLoadedData();

    if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))
        m_stringTable = static_cast<DLCLocalisationFile*>(pack->getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))->getStringTable();

    loadIcon();
    loadName();
    loadDescription();
}

DLCTexturePack::~DLCTexturePack()
{
    delete m_archiveFile;
    m_archiveFile = NULL;
}

void DLCTexturePack::loadIcon()
{
    if (m_dlcPack != NULL && m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Texture, L"icon", true, true))
    {
        DLCFile* file = m_dlcPack->getFile(DLCManager::eDLCType_Texture, L"icon", true, true);
        m_pIconData = file->getData(m_iconSize);
    }
    else
        AbstractTexturePack::loadIcon();
}

void DLCTexturePack::loadName()
{
    std::wstring name = L"";
    if (m_stringTable != NULL)
    {
        if ((m_dlcPack->GetPackId() & 0x400) != 0)
            m_worldName = m_stringTable->getString(L"IDS_WORLD_NAME");
        name = m_stringTable->getString(L"IDS_DISPLAY_NAME");
        if (name.empty())
            name = m_stringTable->getString(L"title");
    }
    m_name = name;
}

void DLCTexturePack::loadDescription()
{
    std::wstring description = L"";
    if (m_stringTable != NULL)
    {
        description = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
        if (description.empty())
            description = m_stringTable->getString(L"description");
    }
    m_desc1 = description;
}

std::wstring DLCTexturePack::getResource(const std::wstring& fileName)
{
    return L"description";
}

DLCPack* DLCTexturePack::getDLCPack()
{
    return m_loadedPack;
}

std::wstring DLCTexturePack::getDesc1()
{
    std::wstring description = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
    if (description.empty())
        description = m_stringTable->getString(L"description");
    return description;
}

std::wstring DLCTexturePack::getName()
{
    std::wstring name = m_stringTable->getString(L"IDS_DISPLAY_NAME");
    if (name.empty())
        name = m_stringTable->getString(L"title");
    return name;
}

std::wstring DLCTexturePack::getWorldName()
{
    return m_stringTable->getString(L"IDS_WORLD_NAME");
}

void DLCTexturePack::loadComparison()
{
    if (m_dlcPack != NULL && m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Texture, L"comparison", true, true))
    {
        DLCFile* file = m_dlcPack->getFile(DLCManager::eDLCType_Texture, L"comparison", true, true);
        m_pComparisonData = file->getData(m_comparisonSize);
    }
}

InputStream* DLCTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    return NULL;
}

bool DLCTexturePack::hasFile(const std::wstring& fileName)
{
    return m_loadedPack != NULL && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Texture, fileName, true, true);
}

bool DLCTexturePack::isTerrainUpdateCompatible()
{
    return true;
}

std::wstring DLCTexturePack::getPath(bool bTitleUpdate, const char* pFileName)
{
    return L"res/";
}

std::wstring DLCTexturePack::getAnimationString(const std::wstring& fileName,
                                                const std::wstring& animationName,
                                                const std::wstring& defaultString)
{
    std::wstring path = L"res/" + fileName;
    path += L"/";
    path += animationName;
    if (m_loadedPack != NULL && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Texture, path, true, true))
    {
        DLCFile* file = m_loadedPack->getFile(DLCManager::eDLCType_Texture, path, true, true);
        unsigned int size = 0;
        unsigned char* data = file->getData(size);
        std::wstring result((wchar_t*)data, (wchar_t*)data + size / sizeof(wchar_t));
        delete[] data;
        return result;
    }
    return L"";
}

BufferedImage* DLCTexturePack::getImageResource(const std::wstring& fileName, bool bSingleImage, bool bTitleUpdate, const std::wstring& path)
{
    if (m_loadedPack != NULL)
        return new BufferedImage(m_loadedPack, L"/" + fileName, bSingleImage);
    return AbstractTexturePack::getImageResource(fileName, bSingleImage, bTitleUpdate, path);
}

void DLCTexturePack::loadMaterialTable()
{
    if (m_loadedPack != NULL && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Material, L"entityMaterials_bin", true, true))
    {
        DLCFile* file = m_loadedPack->getFile(DLCManager::eDLCType_Material, L"entityMaterials_bin", true, true);
        m_pMaterialTable = static_cast<DLCMaterialFileAccess*>(file)->getMaterialTable();
        m_materialTableLoaded = 0;
    }
    else
    {
        if (m_materialTableLoaded == 0)
            m_pMaterialTable = NULL;
        AbstractTexturePack::loadMaterialTable();
        m_materialTableLoaded = 1;
    }
}

void DLCTexturePack::loadColourTable()
{
    if (m_loadedPack != NULL && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_ColourTable, L"colours.col", true, true))
    {
        DLCFile* file = m_loadedPack->getFile(DLCManager::eDLCType_ColourTable, L"colours.col", true, true);
        unsigned int size = 0;
        unsigned char* data = file->getData(size);
        if (m_pColourTable == NULL)
            m_pColourTable = new ColourTable(data, size);
        else
            m_pColourTable->loadColoursFromData(data, size);
        m_colourTableLoaded = 0;
        delete[] data;
    }
    else
    {
        if (m_colourTableLoaded == 0)
            m_pColourTable = NULL;
        AbstractTexturePack::loadColourTable();
        m_colourTableLoaded = 1;
    }
}

void DLCTexturePack::loadStructureManager()
{
    delete m_pStructureManager;
    m_pStructureManager = new StructureManager(NULL);
}

bool DLCTexturePack::hasData()
{
    return m_hasData != 0;
}

bool DLCTexturePack::hasValidData()
{
    return m_hasValidData != 0;
}

bool DLCTexturePack::isLoadingData()
{
    return m_loadingData != 0;
}

void DLCTexturePack::loadCustomModels()
{
    if (m_loadedPack == NULL || !m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Models, L"models_bin", true, true))
        return;
    m_customModels.clear();
    DLCModelsFile* file = static_cast<DLCModelsFile*>(m_loadedPack->getFile(DLCManager::eDLCType_Models, L"models_bin", true, true));
    DataInputStream stream(new ByteArrayInputStream(file->getData()));
    int version = stream.readInt();
    int count = stream.readInt();
    for (int i = 0; i < count; ++i)
    {
        std::wstring name = stream.readUTF();
        m_customModels[name].readModel(&stream, version);
    }
}

unsigned long long DLCTexturePack::loadBehaviours()
{
    if (m_loadedPack == NULL || !m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Behaviours, L"behaviours_bin", true, true))
        return 0;
    DLCBehavioursFile* file = static_cast<DLCBehavioursFile*>(m_loadedPack->getFile(DLCManager::eDLCType_Behaviours, L"behaviours_bin", true, true));
    DataInputStream stream(new ByteArrayInputStream(file->getData()));
    stream.readInt();
    int count = stream.readInt();
    for (int i = 0; i < count; ++i)
        RiderPositionOverrides::ReadOverrides(&stream);
    return 0;
}

unsigned int DLCTexturePack::packMounted(void* data, int result, unsigned int mountIndex, unsigned int deviceId)
{
    DLCTexturePack* pack = static_cast<DLCTexturePack*>(data);
    if (pack == NULL)
        return 0;
    pack->m_loadingData = 0;
    if (result == 0)
    {
        pack->m_loadedPack = new DLCPack(pack->m_dlcPack->getName(), 0);
        pack->m_loadedPack->SetDLCMountIndex(mountIndex);
        pack->m_loadedPack->SetDLCDeviceID((int)deviceId);
        pack->m_hasValidData = 1;
        pack->m_hasData = 1;
        pack->loadUI();
        pack->loadMaterialTable();
        pack->loadColourTable();
        pack->loadCustomModels();
    }
    else
        pack->m_hasValidData = 0;
    return (unsigned int)result;
}

void DLCTexturePack::loadData()
{
    if (m_dlcPack == NULL)
        return;
    m_loadingData = 1;
    if ((int)m_dlcPack->GetDLCMountIndex() < 0)
    {
        m_loadingData = 0;
        m_hasData = 1;
        m_hasValidData = 1;
        loadUI();
        return;
    }
    packMounted(this, 0, m_dlcPack->GetDLCMountIndex(), (unsigned int)m_dlcPack->GetDLCDeviceID());
}

void DLCTexturePack::loadUI()
{
    if (m_archiveFile != NULL && m_archiveFile->hasFile(L"skin_swf"))
        g_ConsoleUIController.ReloadSkin();
    else
        AbstractTexturePack::loadDefaultUI();
    m_uiLoaded = 1;
    AbstractTexturePack::loadUI();
    if (!hasAudio())
        g_ConsoleUIController.IsReloadingSkin();
}

void DLCTexturePack::unloadUI()
{
    if (m_uiLoaded != 0)
        setHasAudio(false);
    AbstractTexturePack::unloadUI();
    g_DLCManager.removePack(m_loadedPack);
    m_loadedPack = NULL;
    delete m_archiveFile;
    m_archiveFile = NULL;
    m_uiLoaded = 0;
    m_hasData = 0;
}

void DLCTexturePack::unloadAudio()
{
}

ArchiveFile* DLCTexturePack::getArchiveFile()
{
    return m_archiveFile;
}

unsigned int DLCTexturePack::getDLCParentPackId()
{
    return m_dlcPack->GetParentPackId();
}

void DLCTexturePack::getDLCInfoParentPack()
{
    m_dlcPack->GetParentPack();
}

unsigned int DLCTexturePack::getDLCSubPackId()
{
    return m_dlcPack->GetPackId() >> 24;
}
