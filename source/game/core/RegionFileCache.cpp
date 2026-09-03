#include "game/core/RegionFileCache.hpp"

#include "game/core/RegionFile.hpp"
#include "game/data/ConsoleSaveFile.hpp"

void MemSect(int section);
template<typename T> std::wstring _toString(T value);

static RegionFileCache gRegionFileCache;

void RegionFileCache::_clear()
{
    for (RegionFileMap::iterator it = mRegionFiles.begin(); it != mRegionFiles.end(); ++it)
    {
        RegionFile* regionFile = it->second;

        if (regionFile != NULL)
        {
            regionFile->close();
        }

        delete regionFile;
    }

    mRegionFiles.clear();
}

RegionFileCache::~RegionFileCache()
{
    _clear();
}

bool RegionFileCache::useSplitSaves(ESavePlatform platform)
{
    return platform == eSavePlatform_NX || platform == eSavePlatform_XB1 || platform == eSavePlatform_PS4;
}

File RegionFileCache::getFilepath(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z)
{
    MemSect(31);

    File filepath;

    if (useSplitSaves(saveFile->getSavePlatform()))
    {
        filepath = File(folder + std::wstring(L"r.") + _toString(x >> 4) + L"." + _toString(z >> 4) + L".mcr");
    }
    else
    {
        filepath = File(folder + std::wstring(L"r.") + _toString(x >> 5) + L"." + _toString(z >> 5) + L".mcr");
    }

    MemSect(0);

    return filepath;
}

RegionFile* RegionFileCache::_getRegionFile(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z)
{
    File filepath = getFilepath(saveFile, folder, x, z);

    RegionFileMap::iterator it = mRegionFiles.find(filepath);

    if (it != mRegionFiles.end() && it->second != NULL)
    {
        return it->second;
    }

    RegionFile* regionFile = new RegionFile(saveFile, &filepath);
    mRegionFiles[filepath] = regionFile;

    return regionFile;
}

int RegionFileCache::_getSizeDelta(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z)
{
    RegionFile* regionFile = _getRegionFile(saveFile, folder, x, z);
    return regionFile != NULL ? regionFile->getSizeDelta() : 0;
}

DataInputStream* RegionFileCache::_getChunkDataInputStream(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z, bool* pbNewSaveFormat)
{
    RegionFile* regionFile = _getRegionFile(saveFile, folder, x, z);

    if (useSplitSaves(saveFile->getSavePlatform()))
    {
        return regionFile->getChunkDataInputStream(x & 15, z & 15, pbNewSaveFormat);
    }

    return regionFile->getChunkDataInputStream(x & 31, z & 31, pbNewSaveFormat);
}

DataOutputStream* RegionFileCache::_getChunkDataOutputStream(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z, bool bNewSaveFormat)
{
    RegionFile* regionFile = _getRegionFile(saveFile, folder, x, z);

    if (useSplitSaves(saveFile->getSavePlatform()))
    {
        return regionFile->getChunkDataOutputStream(x & 15, z & 15, bNewSaveFormat);
    }

    return regionFile->getChunkDataOutputStream(x & 31, z & 31, bNewSaveFormat);
}

File RegionFileCache::fjGetFilepath(ConsoleSaveFile* saveFile, const std::wstring& folder, int x, int z)
{
    return gRegionFileCache.getFilepath(saveFile, folder, x, z);
}
