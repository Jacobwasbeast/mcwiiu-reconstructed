#include "game/client/render/FolderTexturePack.hpp"

#include <string>

#include "game/core/File.hpp"
#include "game/core/InputStream.hpp"

FolderTexturePack::~FolderTexturePack()
{
    return;
}

InputStream* FolderTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    std::wstring strPath = L"";
    strPath = L"Common\\DummyTexturePack\\res";

    std::wstring strFullPath = strPath + fileName;
    return InputStream::getResourceAsStream(strFullPath);
}

bool FolderTexturePack::hasFile(const std::wstring& fileName)
{
    const std::wstring path = getPath(false, NULL) + fileName;
    File file(path);
    return file.exists() && file.isFile();
}

bool FolderTexturePack::isTerrainUpdateCompatible()
{
    return true;
}

std::wstring FolderTexturePack::getPath(bool bTitleUpdate, const char* pFileName)
{
    std::wstring path = L"Common\\" + m_pFile->getPath() + L"\\";
    if (bTitleUpdate)
        path = L"TitleUpdate\\" + m_pFile->getPath() + L"\\";
    if (pFileName != NULL)
    {
        std::string name(pFileName);
        path += std::wstring(name.begin(), name.end());
    }
    return path;
}

void FolderTexturePack::loadUI()
{
}

void FolderTexturePack::unloadUI()
{
}
