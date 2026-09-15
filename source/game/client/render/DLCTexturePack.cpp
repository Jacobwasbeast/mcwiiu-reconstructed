#include "game/client/render/DLCTexturePack.hpp"

#include <string>

#include "game/client/render/DLCModelsFile.hpp"
#include "game/client/audio/DLCAudioFile.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/core/ArchiveFile.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCBehavioursFile.hpp"
#include "game/core/DLCColourTableFile.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/data/StringTable.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/DLCMaterialFile.hpp"
#include "game/world/block/MaterialTable.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"

extern CStorage* gStorage;

void Sleep(unsigned int milliseconds);

DLCTexturePack::DLCTexturePack(unsigned int id, DLCPack* pack, TexturePack* parentPack)
    : AbstractTexturePack(id, NULL, pack->getName(), parentPack)
    , m_dlcPack(pack)
    , m_loadedPack(NULL)
    , m_stringTable(NULL)
    , m_uiLoaded(false)
    , m_loadingData(false)
    , m_hasData(false)
    , m_hasValidData(false)
    , m_colourTableLoaded(true)
    , m_materialTableLoaded(true)
    , m_archiveFile(NULL)
{
    if (gApp.getLevelGenerationOptions() != NULL)
        gApp.getLevelGenerationOptions()->setLoadedData();

    if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))
        m_stringTable = static_cast<DLCLocalisationFile*>(pack->getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))->getStringTable();

    loadIcon();
    loadName();
    loadDescription();
}

DLCTexturePack::~DLCTexturePack()
{
    delete m_archiveFile;
}

void DLCTexturePack::loadIcon()
{
    if (m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Texture, L"icon", true, true))
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
        if (name == L"")
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
        if (description == L"")
            description = m_stringTable->getString(L"description");
    }
    m_desc1 = description;
}

std::wstring DLCTexturePack::getResource(const std::wstring& fileName)
{
    return L"";
}

DLCPack* DLCTexturePack::getDLCPack()
{
    return m_dlcPack;
}

std::wstring DLCTexturePack::getDesc1()
{
    std::wstring description = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
    if (description == L"")
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
    if (m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Texture, L"comparison", true, true))
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
    std::wstring result = L"";
    std::wstring path = L"res/" + defaultString + fileName;
    if (hasFile(path))
    {
        DLCFile* file = m_loadedPack->getFile(DLCManager::eDLCType_Texture, path, true, true);
        result = file->getParameterAsString(DLCManager::eDLCParameterType_Anim, animationName);
    }
    return result;
}

BufferedImage* DLCTexturePack::getImageResource(const std::wstring& fileName, bool bSingleImage, bool bTitleUpdate, const std::wstring& path)
{
    if (m_loadedPack != NULL)
        return new BufferedImage(m_loadedPack, L"/" + fileName, bSingleImage);
    return m_pParentPack->getImageResource(fileName, bSingleImage, bTitleUpdate, path);
}

void DLCTexturePack::loadMaterialTable()
{
    if (m_loadedPack != NULL && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Material, L"entityMaterials_bin", true, true))
    {
        DLCMaterialFile* file = static_cast<DLCMaterialFile*>(m_loadedPack->getFile(DLCManager::eDLCType_Material, L"entityMaterials_bin", true, true));
        m_pMaterialTable = file->m_pMaterialTable;
        m_materialTableLoaded = false;
    }
    else
    {
        if (!m_materialTableLoaded)
            m_pMaterialTable = NULL;
        AbstractTexturePack::loadMaterialTable();
        m_materialTableLoaded = true;
    }
}

void DLCTexturePack::loadColourTable()
{
    if (m_loadedPack != NULL && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_ColourTable, L"colours.col", true, true))
    {
        DLCColourTableFile* file = static_cast<DLCColourTableFile*>(m_loadedPack->getFile(DLCManager::eDLCType_ColourTable, L"colours.col", true, true));
        m_pColourTable = file->m_pColourTable;
        m_colourTableLoaded = false;
    }
    else
    {
        if (!m_colourTableLoaded)
            m_pColourTable = NULL;
        AbstractTexturePack::loadDefaultColourTable();
        m_colourTableLoaded = true;
    }
    AbstractTexturePack::loadDefaultHTMLColourTable();
}

void DLCTexturePack::loadStructureManager()
{
    delete m_pStructureManager;
    m_pStructureManager = new StructureManager(NULL);
}

bool DLCTexturePack::hasData()
{
    return m_hasData;
}

bool DLCTexturePack::hasValidData()
{
    return m_hasValidData;
}

bool DLCTexturePack::isLoadingData()
{
    return m_loadingData;
}

void DLCTexturePack::loadCustomModels()
{
    if (m_loadedPack == NULL || !m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Models, L"models_bin", true, true))
        return;
    m_customModels.clear();
    DLCModelsFile* file = static_cast<DLCModelsFile*>(m_loadedPack->getFile(DLCManager::eDLCType_Models, L"models_bin", true, true));
    ByteArrayInputStream byteStream(file->getData());
    DataInputStream stream(&byteStream);
    int version = stream.readInt();
    int count = stream.readInt();
    for (int i = 0; i < count; ++i)
    {
        std::wstring name = stream.readUTF();
        m_customModels[name].readModel(&stream, version);
    }
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

void DLCTexturePack::loadBehaviours()
{
    if (m_loadedPack == NULL || !m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Behaviours, L"behaviours_bin", true, true))
        return;
    DLCBehavioursFile* file = static_cast<DLCBehavioursFile*>(m_loadedPack->getFile(DLCManager::eDLCType_Behaviours, L"behaviours_bin", true, true));
    ByteArrayInputStream byteStream(file->getData());
    DataInputStream stream(&byteStream);
    stream.readInt();
    int count = stream.readInt();
    for (int i = 0; i < count; ++i)
        RiderPositionOverrides::ReadOverrides(&stream);
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

int DLCTexturePack::packMounted(void* data, int userIndex, unsigned int result, unsigned int licenseMask)
{
    DLCTexturePack* pack = static_cast<DLCTexturePack*>(data);
    pack->m_loadingData = false;
    if (result == 0)
    {
        pack->m_loadedPack = new DLCPack(pack->m_dlcPack->getName(), licenseMask);
        pack->setHasAudio(false);
        pack->m_hasValidData = true;

        unsigned int numItemsAdded = 0;
        std::wstring dataPath = pack->m_dlcPack->getFullDataPath();
        if (dataPath.empty())
        {
            pack->hasAudio();
        }
        else
        {
            std::wstring fullDataPath = gApp.getFilePath(
                pack->m_dlcPack->GetPackId(), dataPath, true, L"TPACK\\");
            bool loaded = gApp.mDLCManager.readDLCDataFile(numItemsAdded, fullDataPath,
                                                       pack->m_loadedPack,
                                                       pack->m_dlcPack->GetPackId(), false);
            if (loaded)
            {
                if (pack->m_loadedPack != NULL)
                {
                    File mediaArchive(gApp.getFilePath(
                        pack->m_dlcPack->GetPackId(), L"media.arc", true, L"TPACK\\"));
                    if (mediaArchive.exists())
                    {
                        delete pack->m_archiveFile;
                        pack->m_archiveFile = new ArchiveFile(mediaArchive);
                    }

                    DLCPack* parentPack = pack->m_dlcPack->GetParentPack();
                    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
                    if (options != NULL &&
                        options->getParentPackId() == pack->m_dlcPack->GetPackId() &&
                        !options->hasLoadedData())
                    {
                        LevelGenerationOptions::loadGameRulesHeaders(parentPack,
                                                                     pack->m_stringTable,
                                                                     options);
                        if (options->requiresBaseSave() && !options->getBaseSavePath().empty())
                        {
                            File baseSave(gApp.getFilePath(
                                pack->m_dlcPack->GetPackId(), options->getBaseSavePath(),
                                true, L"TPACK\\"));
                            options->setBaseSaveData(baseSave);
                        }
                    }

                    if (parentPack->getDLCItemsCount(DLCManager::eDLCType_Audio) != 0)
                    {
                        DLCAudioFile* audioFile = static_cast<DLCAudioFile*>(
                            parentPack->getFile(DLCManager::eDLCType_Audio, 0, true, true));
                        pack->setHasAudio(true);

                        int count[DLCAudioFile::eAudioType_NumTypes];
                        int first[DLCAudioFile::eAudioType_NumTypes];
                        int last[DLCAudioFile::eAudioType_NumTypes];
                        int total = 0;
                        int i;
                        for (i = 0; i < DLCAudioFile::eAudioType_NumTypes; ++i)
                            count[i] = audioFile->GetCountofType((DLCAudioFile::EAudioType)i);

                        for (i = 0; i < 3; ++i)
                        {
                            first[i] = total;
                            total += count[i];
                            last[i] = total - 1;
                        }
                        for (i = 3; i < 8; ++i)
                        {
                            if (count[i] != 0)
                            {
                                first[i] = total;
                                total += count[i];
                            }
                            else
                            {
                                first[i] = 0;
                            }
                            last[i] = first[i] + count[i] - 1;
                        }
                        if (count[8] != 0)
                        {
                            first[8] = total;
                            total += count[8];
                            last[8] = total - 1;
                        }
                        else
                        {
                            first[8] = first[3];
                            last[8] = last[3];
                        }

                        Minecraft::GetInstance()->mSoundEngine->SetStreamingSounds(
                            first[0], last[0], first[1], last[1], first[2], last[2],
                            first[3], last[3], first[4], last[4], first[5], last[5],
                            first[6], last[6], first[7], last[7], first[8], last[8], total);
                    }
                }
            }
            else
            {
                delete pack->m_loadedPack;
                pack->m_loadedPack = NULL;
            }

            pack->loadColourTable();
            pack->loadMaterialTable();
            pack->loadCustomModels();
            pack->loadBehaviours();
            pack->hasAudio();
        }
    }
    else
    {
        pack->loadColourTable();
        pack->loadMaterialTable();
        for (int attempts = 200; gMinecraftServer == NULL && attempts > 0; --attempts)
            Sleep(10);

        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        if (options != NULL)
            options->setLoadedData();
    }

    pack->m_hasData = true;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL && options->getParentPackId() == pack->m_dlcPack->GetPackId())
        options->setLoadedData();
    gApp.StartReloadingTexturePack();
    return 0;
}

void DLCTexturePack::loadData()
{
    int mountIndex = (int)m_dlcPack->GetDLCMountIndex();
    if (mountIndex < 0)
    {
        loadColourTable();
        loadMaterialTable();
        m_hasData = true;
        if (gApp.getLevelGenerationOptions() != NULL)
            gApp.getLevelGenerationOptions()->setLoadedData();
        gApp.StartReloadingTexturePack();
    }
    else
    {
        int result = gStorage->MountInstalledDLC(gProfile->GetPrimaryPad(),
                                                 (unsigned int)mountIndex,
                                                 packMounted, this, "TPACK");
        if (result == 0x3e5)
        {
            m_loadingData = true;
            return;
        }

        loadColourTable();
        loadMaterialTable();
        m_hasData = true;
        if (gApp.getLevelGenerationOptions() != NULL)
            gApp.getLevelGenerationOptions()->setLoadedData();
    }
}

void DLCTexturePack::loadUI()
{
    if (m_archiveFile != NULL && m_archiveFile->hasFile(L"skin_swf"))
        gConsoleUIController.ReloadSkin();
    else
        AbstractTexturePack::loadDefaultUI();
    m_uiLoaded = true;
    AbstractTexturePack::loadUI();
    if (!hasAudio())
        gConsoleUIController.IsReloadingSkin();
}

void DLCTexturePack::unloadUI()
{
    if (m_uiLoaded)
        setHasAudio(false);
    AbstractTexturePack::unloadUI();
    gApp.mDLCManager.removePack(m_loadedPack);
    m_loadedPack = NULL;
    delete m_archiveFile;
    m_archiveFile = NULL;
    m_uiLoaded = false;
    m_hasData = false;
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

DLCPack* DLCTexturePack::getDLCInfoParentPack()
{
    return m_dlcPack->GetParentPack();
}

unsigned int DLCTexturePack::getDLCSubPackId()
{
    return m_dlcPack->GetPackId() >> 24;
}
