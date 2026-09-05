#ifndef GAME_CLIENT_RENDER_DLCTEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_DLCTEXTUREPACK_HPP

#include <string>

#include "game/client/render/AbstractTexturePack.hpp"

class ArchiveFile;
class DLCPack;
class StringTable;

class DLCTexturePack : public AbstractTexturePack
{
public:
    DLCTexturePack(unsigned int id, DLCPack* pack, TexturePack* parentPack);
    virtual ~DLCTexturePack();

    virtual void loadIcon();
    virtual void loadName();
    virtual void loadDescription();
    virtual std::wstring getResource(const std::wstring& fileName);
    virtual DLCPack* getDLCPack();
    virtual std::wstring getDesc1();
    virtual std::wstring getName();
    virtual std::wstring getWorldName();
    virtual void loadComparison();
    virtual InputStream* getResourceImplementation(const std::wstring& fileName);
    virtual bool hasFile(const std::wstring& fileName);
    virtual bool isTerrainUpdateCompatible();
    virtual std::wstring getPath(bool bTitleUpdate, const char* pFileName);
    virtual std::wstring getAnimationString(const std::wstring& fileName,
                                            const std::wstring& animationName,
                                            const std::wstring& defaultString);
    virtual BufferedImage* getImageResource(const std::wstring& fileName,
                                             bool bSingleImage,
                                             bool bTitleUpdate,
                                             const std::wstring& path);
    virtual void loadMaterialTable();
    virtual void loadColourTable();
    virtual void loadStructureManager();
    virtual bool hasData();
    virtual bool hasValidData();
    virtual bool isLoadingData();
    virtual void loadCustomModels();
    unsigned long long loadBehaviours();
    static unsigned int packMounted(void* data, int result, unsigned int mountIndex, unsigned int deviceId);
    virtual void loadData();
    virtual void loadUI();
    virtual void unloadUI();
    virtual void unloadAudio();
    virtual ArchiveFile* getArchiveFile();
    virtual unsigned int getDLCParentPackId();
    void getDLCInfoParentPack();
    virtual unsigned int getDLCSubPackId();
    int GetDLCDeviceID();

private:
    DLCPack* m_dlcPack;
    DLCPack* m_loadedPack;
    StringTable* m_stringTable;
    unsigned char m_uiLoaded;
    unsigned char m_loadingData;
    unsigned char m_hasData;
    unsigned char m_hasValidData;
    unsigned char m_materialTableLoaded;
    unsigned char m_colourTableLoaded;
    unsigned short m_padding;
    ArchiveFile* m_archiveFile;
};

#endif
