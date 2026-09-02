#ifndef GAME_CLIENT_RENDER_FOLDERTEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_FOLDERTEXTUREPACK_HPP

#include <string>

#include "game/client/render/AbstractTexturePack.hpp"

class File;
class InputStream;
class TexturePack;

class FolderTexturePack : public AbstractTexturePack
{
public:
    FolderTexturePack(unsigned int id,
                      const std::wstring& name,
                      File* pFolder,
                      TexturePack* pParentPack);
    virtual ~FolderTexturePack();

    virtual bool isTerrainUpdateCompatible();
    virtual std::wstring getPath(bool bTitleUpdate, const char* pFileName);
    virtual void loadUI();
    virtual void unloadUI();
    virtual InputStream* getResourceImplementation(const std::wstring& fileName);
    virtual bool hasFile(const std::wstring& fileName);
};

#endif
