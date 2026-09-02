#include "game/core/MemoryTracker.hpp"

#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/ByteBuffer.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"

static boost::unordered_map<int, int> mapDisplayLists;
static std::vector<int> mapTextureObjects;

int MemoryTracker::genLists(int count)
{
    int list = GlStateManager::genLists(count);
    mapDisplayLists.insert(std::make_pair(list, count));
    return list;
}

void MemoryTracker::releaseLists(int list)
{
    boost::unordered_map<int, int>::iterator entry = mapDisplayLists.find(list);
    if (entry != mapDisplayLists.end())
    {
        GlStateManager::deleteLists(list, entry->second);
        mapDisplayLists.erase(entry);
    }
}

int MemoryTracker::genTextures()
{
    int texture = GlStateManager::genTexture();
    mapTextureObjects.push_back(texture);
    return texture;
}

void MemoryTracker::releaseTextures()
{
    for (unsigned int i = 0; i < mapTextureObjects.size(); ++i)
    {
        GlStateManager::deleteTexture(mapTextureObjects.at(i));
    }

    mapTextureObjects.clear();
}

void MemoryTracker::release()
{
    for (boost::unordered_map<int, int>::iterator entry = mapDisplayLists.begin(); entry != mapDisplayLists.end(); ++entry)
    {
        GlStateManager::deleteLists(entry->first, entry->second);
    }

    mapDisplayLists.clear();
    releaseTextures();
}

ByteBuffer* MemoryTracker::createByteBuffer(int capacity)
{
    return ByteBuffer::allocate(capacity);
}

IntBuffer* MemoryTracker::createIntBuffer(int capacity)
{
    return createByteBuffer(capacity << 2)->asIntBuffer();
}

FloatBuffer* MemoryTracker::createFloatBuffer(int capacity)
{
    return createByteBuffer(capacity << 2)->asFloatBuffer();
}
