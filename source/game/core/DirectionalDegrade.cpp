#include "game/core/DirectionalDegrade.hpp"

#include <algorithm>

#include "game/core/CachedSortFunction.hpp"
#include "game/core/PerlinNoise.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"

DirectionalDegrade::DirectionalDegrade()
    : mDirection(5)
{
}

DirectionalDegrade::~DirectionalDegrade()
{
}

void DirectionalDegrade::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    (void)saveInterface;
    (void)extraAttributeCount;
}

void DirectionalDegrade::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"dir")
    {
        if (value == L"down")
        {
            mDirection = 1;
        }
        else if (value == L"up")
        {
            mDirection = 2;
        }
        else if (value == L"north")
        {
            mDirection = 3;
        }
        else if (value == L"south")
        {
            mDirection = 4;
        }
        else if (value == L"west")
        {
            mDirection = 5;
        }
        else if (value == L"east")
        {
            mDirection = 6;
        }
        return;
    }

    DegradationRuleDefinition::addAttribute(name, value);
}

void DirectionalDegrade::ClearCaches()
{
    mBlocks.clear();
}

void DirectionalDegrade::NextBlocks(BlockDegradeRoutine::LayerCache* layerCache,
                                    arrayWithLength<BlockPos>& blocks)
{
    if (mBlocks.empty())
    {
        Iterator<BlockPos*>* iterator = layerCache->iterator();
        BlockPos* position = iterator->next();
        while (iterator->hasNext())
        {
            mBlocks.push_back(*position);
            iterator->next();
        }
        delete iterator;

        AABB* bounds = layerCache->getCurrentBounds();
        BlockPos min(bounds->mMinX, bounds->mMinY, bounds->mMinZ);
        BlockPos size(bounds->mMaxX - bounds->mMinX,
                      bounds->mMaxY - bounds->mMinY,
                      bounds->mMaxZ - bounds->mMinZ);
        size = size.offset(BlockPos(1, 1, 1));

        Random random;
        PerlinNoise noise(&random, 3);
        arrayWithLength<double> values;
        values = noise.getRegion(values, min.x, min.y, min.z,
                                 size.x, size.y, size.z, 5.0, 5.0, 5.0);

        CachedSortFunction sortFunction(values, bounds);
        std::sort(mBlocks.begin(), mBlocks.end(), sortFunction);
        delete[] values.data;
    }

    unsigned int blockCount = 0;
    std::vector<BlockPos>::iterator position = mBlocks.begin();
    while (blockCount < blocks.length && position != mBlocks.end())
    {
        if (layerCache->inLayer(*position))
        {
            blocks.data[blockCount] = *position;
            ++blockCount;
        }
        ++position;
    }
}

ConsoleGameRules::EGameRuleType DirectionalDegrade::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(35);
}
