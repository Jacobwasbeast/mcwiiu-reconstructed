#include "game/world/biome/BiomeCache.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/core/MemSect.hpp"

BiomeCache::BiomeCache(BiomeSource* biomeSource)
    : mBiomeSource(biomeSource), mLastUpdate(0)
{
    InitializeCriticalSection(&mCriticalSection);
}

BiomeCache::~BiomeCache()
{
    for (std::vector<Area*>::iterator it = mAreaList.begin(); it != mAreaList.end(); ++it)
    {
        delete *it;
    }

    DeleteCriticalSection(&mCriticalSection);
}

BiomeCache::Area* BiomeCache::getBlockAt(int x, int z)
{
    EnterCriticalSection(&mCriticalSection);

    int chunkX = x >> 4;
    int chunkZ = z >> 4;
    long long key = (static_cast<long long>(chunkZ) << 32) | static_cast<unsigned int>(chunkX);
    AreaMap::iterator found = mAreas.find(key);
    Area* area;

    if (found == mAreas.end())
    {
        MemSect(0x30);
        area = new Area(chunkX, chunkZ, this);
        mAreas[key] = area;
        mAreaList.push_back(area);
        MemSect(0);
    }
    else
    {
        area = found->second;
    }

    area->mLastUpdate = static_cast<long long>(g_pApp->getAppTime());
    LeaveCriticalSection(&mCriticalSection);
    return area;
}

Biome* BiomeCache::getBiome(int x, int z, Biome* fallback)
{
    Biome* biome = getBlockAt(x, z)->getBiome(x, z);
    return biome == NULL ? fallback : biome;
}

void BiomeCache::update()
{
    EnterCriticalSection(&mCriticalSection);

    long long now = static_cast<long long>(g_pApp->getAppTime());
    if (now - mLastUpdate >= 7500)
    {
        mLastUpdate = now;

        std::vector<Area*>::iterator it = mAreaList.begin();
        while (it != mAreaList.end())
        {
            Area* area = *it;
            if (now - area->mLastUpdate >= 30000)
            {
                long long key = (static_cast<long long>(area->mZ) << 32) |
                                static_cast<unsigned int>(area->mX);
                it = mAreaList.erase(it);
                mAreas.erase(key);
                delete area;
            }
            else
            {
                ++it;
            }
        }
    }

    LeaveCriticalSection(&mCriticalSection);
}

arrayWithLength<Biome*> BiomeCache::getBiomeBlockAt(int x, int z)
{
    Area* area = getBlockAt(x, z);
    arrayWithLength<Biome*> result(area->mBiomeIndices.length, true);

    for (unsigned int i = 0; i < area->mBiomeIndices.length; ++i)
    {
        result.data[i] = Biome::getBiome(area->mBiomeIndices.data[i], Biome::DEFAULT);
    }

    return result;
}

arrayWithLength<unsigned char> BiomeCache::getBiomeIndexBlockAt(int x, int z)
{
    Area* area = getBlockAt(x, z);
    arrayWithLength<unsigned char> result;
    result.data = area->mBiomeIndices.data;
    result.length = area->mBiomeIndices.length;
    return result;
}

BiomeCache::Area::Area(int x, int z, BiomeCache* biomeCache)
    : mBiomeIndices(), mX(x), mZ(z), mLastUpdate(0)
{
    mBiomeIndices.data = new unsigned char[256];
    mBiomeIndices.length = 256;
    biomeCache->mBiomeSource->getBiomeIndexBlock(mBiomeIndices, x << 4, z << 4, 16, 16, false);
}

BiomeCache::Area::~Area()
{
    delete[] mBiomeIndices.data;
}

Biome* BiomeCache::Area::getBiome(int x, int z)
{
    return Biome::getBiome(mBiomeIndices.data[((z & 15) << 4) | (x & 15)], Biome::DEFAULT);
}
