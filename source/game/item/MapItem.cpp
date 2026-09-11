#include "game/item/MapItem.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <typeinfo>

#include "game/core/IconRegister.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"

MapItem::MapItem()
    : ComplexItem()
{
    setStackedByData(true);
}

MapItem::~MapItem()
{
}

static std::wstring mapName(int mapId)
{
    return std::wstring(L"map_") + _toString(mapId);
}

not_null_ptr<ItemInstance> MapItem::setupNewMap(PlayerUID uid, Level* level, int dimension,
                                                double x, double z, unsigned char scale,
                                                bool trackingPosition, bool unlimitedTracking)
{
    BlockPos pos((int)x, 0, (int)z);
    int mapId = level->getAuxValueForMap(uid, dimension, pos, (int)scale);
    not_null_ptr<ItemInstance> itemInstance(new ItemInstance(Items::filledMap, 1, mapId));
    std::wstring name = mapName(mapId);
    boost::shared_ptr<MapItemSavedData> data(new MapItemSavedData(name));
    data->mScale = scale;
    data->setOrigin(x, z, data->mScale);
    data->mDimension = level->mDimension->getType()->getId();
    data->mTrackingPosition = trackingPosition;
    data->mUnlimitedTracking = unlimitedTracking;
    data->setDirty();
    level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
    return itemInstance;
}

boost::shared_ptr<MapItemSavedData> MapItem::getSavedDataIfExists(int mapId, Level* level)
{
    std::wstring name = mapName(mapId);
    boost::shared_ptr<SavedData> saved = level->getSavedData(typeid(MapItemSavedData), name);
    return boost::static_pointer_cast<MapItemSavedData>(saved);
}

boost::shared_ptr<MapItemSavedData> MapItem::getSavedData(int mapId, Level* level)
{
    std::wstring name = mapName(mapId);
    boost::shared_ptr<MapItemSavedData> data = getSavedDataIfExists(mapId, level);
    if (!data)
    {
        data.reset(new MapItemSavedData(name));
        data->setOrigin(0.0, 0.0, 0);
        data->setDirty();
        level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
    }
    return data;
}

boost::shared_ptr<MapItemSavedData> MapItem::getSavedData(not_null_ptr<ItemInstance> itemInstance, Level* level)
{
    return getSavedData(itemInstance->getAuxValue(), level);
}

namespace
{

uint64_t& mapLastUpdateTime(MapItemSavedData* data)
{
    return *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(data) + 136);
}

int& holdingPlayerTick(MapItemSavedData::HoldingPlayer* holding)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(holding) + 44);
}

const BlockState* findMapBlockState(LevelChunk* chunk, Level* level, int x, int y, int z)
{
    while (y > 0)
    {
        const BlockState* state = chunk->getBlockState(x & 15, y, z & 15);
        Block* block = state->getBlock();
        if (block == Blocks::AIR)
        {
            --y;
            continue;
        }
        if (!block->isSolidRender(state))
        {
            --y;
            continue;
        }
        Material* material = state->getMaterial();
        if (material != NULL && material->isLiquid())
        {
            return state;
        }
        int scanY = y;
        while (scanY > 0)
        {
            int belowY = scanY - 1;
            if (belowY < y - 5)
            {
                break;
            }
            const BlockState* below = chunk->getBlockState(x & 15, belowY, z & 15);
            Block* belowBlock = below->getBlock();
            if (belowBlock == Blocks::AIR || !belowBlock->isSolidRender(below))
            {
                break;
            }
            Material* belowMaterial = below->getMaterial();
            if (belowMaterial != NULL && belowMaterial->isLiquid())
            {
                break;
            }
            scanY = belowY;
        }
        return chunk->getBlockState(x & 15, scanY, z & 15);
    }
    return Blocks::AIR->defaultBlockState();
}

int chooseBrightness(double heightDelta, bool isWater, int parity)
{
    if (isWater)
    {
        double sample = (static_cast<double>(parity) - 0.5) * 0.4 + 0.5 * 0.2;
        int brightness = 1;
        if (sample < 0.9)
        {
            brightness = 2;
        }
        if (0.9 < sample)
        {
            brightness = 0;
        }
        return brightness;
    }

    double sample = (static_cast<double>(parity) - 0.5) * 0.2 + heightDelta * 4.0;
    int brightness = 1;
    if (0.6 < sample)
    {
        brightness = 2;
    }
    if (sample < -0.6)
    {
        brightness = 0;
    }
    return brightness;
}

unsigned char sampleNetherMapColor(int worldX, int worldZ, Level* level, MutableBlockPos& pos)
{
    int hashInput = worldX + worldZ * 0x389bf;
    unsigned int hash = static_cast<unsigned int>(hashInput * hashInput * 0x1dd6751 + hashInput * 0xb) >> 20;
    const BlockState* state;
    if ((hash & 1) == 0)
    {
        state = Blocks::bedrock->defaultBlockState();
    }
    else
    {
        state = Blocks::netherrack->defaultBlockState();
    }
    pos.set(worldX, 0, worldZ);
    state = state->fillVirtualBlockStateProperties(level, pos);
    const MaterialColor* color = state->getMapColor(level, pos);
    return static_cast<unsigned char>(color->id * 4 + 1);
}

unsigned char sampleOverworldMapColor(LevelChunk* chunk, Level* level, int worldX, int worldZ, int scale,
                                      MutableBlockPos& pos, int parity)
{
    int counts[256];
    std::memset(counts, 0, sizeof(counts));
    double heightSum = 0.0;
    int heightSamples = 0;
    double baseHeight = 0.0;
    bool hasBaseHeight = false;
    for (int dx = 0; dx < scale; ++dx)
    {
        for (int dz = 0; dz < scale; ++dz)
        {
            int x = worldX + dx;
            int z = worldZ + dz;
            int y = chunk->getHeightmap(x & 15, z & 15);
            const BlockState* state = findMapBlockState(chunk, level, x, y, z);
            pos.set(x, y, z);
            const MaterialColor* color = state->getMapColor(level, pos);
            int colorId = color->id;
            ++counts[colorId];
            if (!hasBaseHeight)
            {
                baseHeight = static_cast<double>(y);
                hasBaseHeight = true;
            }
            heightSum += static_cast<double>(y);
            ++heightSamples;
        }
    }

    int bestColorId = 0;
    int bestCount = 0;
    for (int colorId = 0; colorId < 256; ++colorId)
    {
        if (counts[colorId] > bestCount)
        {
            bestCount = counts[colorId];
            bestColorId = colorId;
        }
    }

    double averageHeight = heightSamples > 0 ? heightSum / static_cast<double>(heightSamples) : baseHeight;
    double heightDelta = (averageHeight - baseHeight) * 0.014;
    bool isWater = MaterialColor::WATER != NULL && bestColorId == MaterialColor::WATER->id;
    int brightness = chooseBrightness(heightDelta, isWater, parity);
    return static_cast<unsigned char>(bestColorId * 4 + brightness);
}

} // namespace

void MapItem::update(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<MapItemSavedData> data)
{
    if (level->mDimension->getType()->getId() != data->mDimension)
    {
        return;
    }

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
    {
        return;
    }

    uint64_t now = System::processTimeInMilliSecs();
    uint64_t lastUpdate = mapLastUpdateTime(data.get());
    if (now < lastUpdate)
    {
        return;
    }
    mapLastUpdateTime(data.get()) = now;

    PIXBeginNamedEvent(0.0f, "Update map");

    int scale = 1 << (data->mScale & 0x3f);
    int mapCenterX = static_cast<int>((entity->mX - static_cast<double>(data->mXCenter)) / static_cast<double>(scale)) + 64;
    int mapCenterZ = static_cast<int>((entity->mZ - static_cast<double>(data->mZCenter)) / static_cast<double>(scale)) + 64;
    int halfWidth = 128 / scale;
    if (level->mDimension->isHasCeiling())
    {
        halfWidth = (halfWidth + (halfWidth < 0 ? 1 : 0)) / 2;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    boost::shared_ptr<MapItemSavedData::HoldingPlayer> holding = data->getHoldingPlayer(player);
    ++holdingPlayerTick(holding.get());

    MutableBlockPos pos(0, 0, 0);
    int halfWidthSq = halfWidth * halfWidth;
    int innerRadiusSq = (halfWidth - 2) * (halfWidth - 2);

    for (int mapX = mapCenterX - halfWidth; mapX <= mapCenterX + halfWidth; ++mapX)
    {
        if (((mapX ^ holdingPlayerTick(holding.get())) & 0xf) != 0)
        {
            continue;
        }

        for (int mapZ = mapCenterZ - halfWidth; mapZ <= mapCenterZ + halfWidth; ++mapZ)
        {
            if (mapX < 0 || mapX >= 128 || mapZ < 0 || mapZ >= 128)
            {
                continue;
            }

            int dx = mapX - mapCenterX;
            int dz = mapZ - mapCenterZ;
            int distSq = dx * dx + dz * dz;
            if (distSq >= halfWidthSq)
            {
                continue;
            }

            int parity = (mapX + mapZ) & 1;
            if (distSq > innerRadiusSq && parity == 0)
            {
                continue;
            }

            int worldX = (mapX - 64) * scale + data->mXCenter;
            int worldZ = (mapZ - 64) * scale + data->mZCenter;
            pos.set(worldX, 0, worldZ);
            LevelChunk* chunk = level->getChunkAt(pos);
            if (!chunk->isReady())
            {
                continue;
            }

            unsigned char colorByte;
            if (level->mDimension->isHasCeiling())
            {
                colorByte = sampleNetherMapColor(worldX, worldZ, level, pos);
            }
            else
            {
                colorByte = sampleOverworldMapColor(chunk, level, worldX, worldZ, scale, pos, parity);
            }

            int index = mapX + mapZ * 128;
            if (data->mColors.data[index] != colorByte)
            {
                data->mColors.data[index] = colorByte;
                data->setDirty(mapX, mapZ);
            }
        }
    }

    PIXEndNamedEvent();
}

void MapItem::renderBiomePreviewMap(Level* level, const not_null_ptr<ItemInstance>& itemInstance)
{
    boost::shared_ptr<MapItemSavedData> data =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(itemInstance, level);
    if (data)
    {
        data->setDirty();
    }
}

void MapItem::inventoryTick(not_null_ptr<ItemInstance> itemInstance, Level* level,
                            boost::shared_ptr<Entity> entity, int slot, bool selected)
{
    boost::shared_ptr<MapItemSavedData> data = getSavedData(itemInstance, level);
    if (data)
    {
        data->setDirty();
    }
}

boost::shared_ptr<Packet> MapItem::getUpdatePacket(not_null_ptr<ItemInstance> itemInstance,
                                                   Level* level, boost::shared_ptr<Player> player)
{
    boost::shared_ptr<MapItemSavedData> data = getSavedData(itemInstance, level);
    if (!data)
    {
        return boost::shared_ptr<Packet>();
    }
    return data->getUpdatePacket(itemInstance, level, player);
}

void MapItem::scaleMap(not_null_ptr<ItemInstance> itemInstance, Level* level, int scale)
{
    boost::shared_ptr<MapItemSavedData> oldData =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(itemInstance, level);
    if (oldData)
    {
        std::wstring name = mapName(level->getFreeAuxValueFor(L"map"));
        boost::shared_ptr<MapItemSavedData> data(new MapItemSavedData(name));
        data->setOrigin(0.0, 0.0, 0);
        data->mScale = scale;
        data->setDirty();
        itemInstance->setAuxValue(level->getFreeAuxValueFor(name));
        level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
    }
}

void MapItem::enableMapTracking(not_null_ptr<ItemInstance> itemInstance, Level* level)
{
    boost::shared_ptr<MapItemSavedData> data =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(itemInstance, level);
    if (data)
    {
        data->mTrackingPosition = true;
        data->setDirty();
    }
}

void MapItem::onCraftedBy(not_null_ptr<ItemInstance> itemInstance, Level* level,
                          boost::shared_ptr<Player> player)
{
    CompoundTag* tag = itemInstance->getTag();
    if (!tag)
    {
        return;
    }
    if (tag->contains(L"map_scale_direction", 99))
    {
        scaleMap(itemInstance, level, tag->getInt(L"map_scale_direction"));
        tag->remove(L"map_scale_direction");
    }
    else if (tag->getBoolean(L"map_tracking_position"))
    {
        enableMapTracking(itemInstance, level);
        tag->remove(L"map_tracking_position");
    }
}

int MapItem::getColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    CompoundTag* display = static_cast<CompoundTag*>(itemInstance->getTagElement(L"display"));
    if (!display || !display->contains(L"MapColor", 99))
    {
        return 0xff46402e;
    }
    return display->getInt(L"MapColor") & 0x00ffffff | 0xff000000;
}

bool MapItem::hasMultipleSpriteLayers()
{
    return true;
}

TextureAtlasSprite* MapItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer == 1)
    {
        return mIcon;
    }
    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

void MapItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mIcon = iconRegister->registerIcon(L"map_filled_markings");
}
