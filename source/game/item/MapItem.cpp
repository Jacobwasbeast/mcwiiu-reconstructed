#include "game/item/MapItem.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <typeinfo>

#include "game/core/IconRegister.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
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
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

MapItem::MapItem()
    : ComplexItem()
{
    setStackedByData(true);
}

MapItem::~MapItem()
{
}

not_null_ptr<ItemInstance> MapItem::setupNewMap(PlayerUID uid, Level* level, int dimension,
                                                double x, double z, unsigned char scale,
                                                bool trackingPosition, bool unlimitedTracking)
{
    BlockPos pos((int)x, 0, (int)z);
    int mapId = level->getAuxValueForMap(uid, dimension, pos, (int)scale);
    not_null_ptr<ItemInstance> itemInstance(new ItemInstance(Items::filledMap, 1, mapId));
    std::wstring name = L"map_" + _toString(itemInstance->getAuxValue());
    boost::shared_ptr<MapItemSavedData> data = getSavedDataIfExists(itemInstance->getAuxValue(), level);
    if (!data)
    {
        data.reset(new MapItemSavedData(name));
        data->mScale = scale;
        data->setOrigin(x, z, data->mScale);
        data->mDimension = level->mDimension->getType()->getId();
        data->mTrackingPosition = trackingPosition;
        data->mUnlimitedTracking = unlimitedTracking;
        level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
    }
    data->setDirty();
    return itemInstance;
}

boost::shared_ptr<MapItemSavedData> MapItem::getSavedDataIfExists(int mapId, Level* level)
{
    std::wstring name = L"map_" + _toString(mapId);
    boost::shared_ptr<SavedData> saved = level->getSavedData(typeid(MapItemSavedData), name);
    return boost::static_pointer_cast<MapItemSavedData>(saved);
}

boost::shared_ptr<MapItemSavedData> MapItem::getSavedData(int mapId, Level* level)
{
    MemSect(0x1f);
    std::wstring name = L"map_" + _toString(mapId);
    MemSect(0);
    boost::shared_ptr<MapItemSavedData> data =
        boost::static_pointer_cast<MapItemSavedData>(level->getSavedData(typeid(MapItemSavedData), name));
    if (!data)
    {
        if (level->mIsClientSide)
        {
            return data;
        }
        data.reset(new MapItemSavedData(name));
        data->mScale = 3;
        data->setOrigin(0.0, 0.0, data->mScale);
        data->mDimension = level->mDimension->getType()->getId();
        data->setDirty();
        level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
    }
    if (data->mScale > 4)
    {
        data->mScale = 4;
    }
    return data;
}

boost::shared_ptr<MapItemSavedData> MapItem::getSavedData(not_null_ptr<ItemInstance> itemInstance, Level* level)
{
    return getSavedData(itemInstance->getAuxValue(), level);
}

void MapItem::update(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<MapItemSavedData> data)
{
    if (level->mDimension->getType()->getId() != data->mDimension)
        return;
    if ((entity->GetType() & EntityType::PLAYER) != EntityType::PLAYER)
        return;

    long long now = System::processTimeInMilliSecs();
    if (now < data->mLastUpdateTime)
        return;
    data->mLastUpdateTime = now;

    PIXBeginNamedEvent(0.0f, "Update map");

    int scale = 1 << (data->mScale & 0x3f);
    int xCenter = data->mXCenter;
    int zCenter = data->mZCenter;
    int playerX = Mth::floor(entity->mX - (double)xCenter) / scale + 64;
    int playerZ = Mth::floor(entity->mZ - (double)zCenter) / scale + 64;
    int radius = 128 / scale;
    if (level->mDimension->isHasCeiling())
        radius /= 2;

    boost::shared_ptr<MapItemSavedData::HoldingPlayer> holding =
        data->getHoldingPlayer(boost::static_pointer_cast<Player>(entity));
    ++holding->mStep;

    MutableBlockPos pos(0, 0, 0);
    for (int x = playerX - radius + 1; x < playerX + radius; ++x)
    {
        if (((x ^ holding->mStep) & 15) != 0)
            continue;

        double lastHeight = 0.0;
        for (int z = playerZ - radius - 1; z < playerZ + radius; ++z)
        {
            if (x < 0 || z < -1 || x >= 128 || z >= 128)
                continue;

            int dx = x - playerX;
            int dz = z - playerZ;
            int distSq = dx * dx + dz * dz;
            int counts[64];
            std::memset(counts, 0, sizeof(counts));

            int worldX = (xCenter / scale + x - 64) * scale;
            int worldZ = (zCenter / scale + z - 64) * scale;
            pos.set(worldX, 0, worldZ);
            LevelChunk* chunk = level->getChunkAt(pos);
            if (chunk->isEmpty())
                continue;

            int localX = worldX & 15;
            int localZ = worldZ & 15;
            int waterDepth = 0;
            double height = 0.0;
            if (level->mDimension->isHasCeiling())
            {
                int hash = worldX + worldZ * 231871;
                hash = hash * hash * 31287121 + hash * 11;
                if (((unsigned int)hash >> 20 & 1) == 0)
                {
                    TypedBoxed<DirtBlock::Variant*> variant(DirtBlock::Variant::DIRT);
                    const BlockState* state = Blocks::dirt->defaultBlockState()->setBoxedValue(DirtBlock::VARIANT, &variant);
                    counts[state->getMapColor(level, pos)->id] += 10;
                }
                else
                {
                    TypedBoxed<StoneBlock::Variant*> variant(StoneBlock::Variant::STONE);
                    const BlockState* state =
                        Blocks::stone->defaultBlockState()->setBoxedValue(StoneBlock::VARIANT, &variant);
                    counts[state->getMapColor(level, pos)->id] += 100;
                }
                height = 100.0;
            }
            else
            {
                for (int sampleX = 0; sampleX < scale; ++sampleX)
                {
                    for (int sampleZ = 0; sampleZ < scale; ++sampleZ)
                    {
                        int y = chunk->getHeightmap(sampleX + localX, sampleZ + localZ) + 1;
                        const BlockState* state = Blocks::air->defaultBlockState();
                        int colorId = 0;
                        if (y > 1)
                        {
                            do
                            {
                                --y;
                                pos.set(sampleX + localX, y, sampleZ + localZ);
                                state = chunk->getBlockState(pos);
                                colorId = state->getMapColor(level, pos)->id;
                            } while (y > 0 && colorId == 0);

                            if (y > 0 && state->getBlock()->mMaterial->isLiquid())
                            {
                                int depthY = y - 4;
                                while (depthY > 0)
                                {
                                    const BlockState* below = chunk->getBlockState(sampleX + localX, depthY, sampleZ + localZ);
                                    depthY -= 4;
                                    waterDepth += 4;
                                    if (depthY <= 0 || !below->getMaterial()->isLiquid())
                                        break;
                                }
                            }
                        }
                        else
                        {
                            state = Blocks::bedrock->defaultBlockState();
                        }
                        height += (double)y / (double)(scale * scale);
                        ++counts[colorId];
                    }
                }
            }

            waterDepth /= scale * scale;
            int parity = (x + z) & 1;
            double shade = (double)(height - lastHeight) * 4.0 / (double)(scale + 4) + ((double)parity - 0.5) * 0.4;
            int brightness = 1;
            if (shade > 0.6)
                brightness = 2;
            if (shade < -0.6)
                brightness = 0;

            int bestColor = 0;
            int bestCount = 0;
            for (int colorId = 0; colorId < 64; ++colorId)
            {
                if (bestCount < counts[colorId])
                {
                    bestColor = colorId;
                    bestCount = counts[colorId];
                }
            }
            const MaterialColor* color = MaterialColor::MATERIAL_COLORS[bestColor];
            if (color == MaterialColor::WATER)
            {
                shade = (double)waterDepth * 0.1 + (double)parity * 0.2;
                brightness = 1;
                if (shade < 0.5)
                    brightness = 2;
                if (shade > 0.9)
                    brightness = 0;
            }
            lastHeight = height;

            if (z >= 0 && distSq < radius * radius && (distSq <= (radius - 2) * (radius - 2) || parity != 0))
            {
                unsigned char value = (unsigned char)(color->id * 4 + brightness);
                if (data->mColors.data[x + z * 128] != value)
                {
                    data->mColors.data[x + z * 128] = value;
                    data->setDirty(x, z);
                }
            }
        }
    }

    PIXEndNamedEvent();
}

void MapItem::renderBiomePreviewMap(Level* level, const not_null_ptr<ItemInstance>& itemInstance)
{
    if (itemInstance->getItem() != Items::filledMap)
    {
        return;
    }

    boost::shared_ptr<MapItemSavedData> data =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(itemInstance, level);
    if (!data)
    {
        return;
    }

    if (level->mDimension->getType()->getId() != data->mDimension)
    {
        return;
    }

    int scale = 1 << (data->mScale & 0x3f);
    int size = scale * 128;
    BiomeSource* biomeSource = level->getBiomeSource();
    arrayWithLength<Biome*> biomes;
    biomeSource->getBiomeBlock(biomes, (data->mXCenter / scale - 64) * scale,
                               (data->mZCenter / scale - 64) * scale, size, size, false);

    int worldX = data->mXCenter - size / 2;
    int worldZ = data->mZCenter - size / 2;

    for (int dx = 0; dx < size; ++dx)
    {
        for (int dz = 0; dz < size; ++dz)
        {
            if (!level->isWithinLevelBounds(worldX + dx, worldZ + dz))
            {
                biomes.data[dz * size + dx] = Biome::DEEP_OCEAN;
            }
        }
    }

    for (int mapX = 1; mapX < 128; ++mapX)
    {
        int sampleRow = mapX * scale;
        int sampleRowAbove = sampleRow - scale;
        int sampleRowBelow = sampleRow + scale;

        for (int mapZ = 1; mapZ < 127; ++mapZ)
        {
            int sampleColMid = mapZ * scale * size;
            int sampleColLeft = sampleColMid - scale * size;
            int sampleColRight = sampleColMid + scale * size;

            Biome* center = biomes.data[sampleRow + sampleColMid];

            int votes = 8;
            if (biomes.data[sampleRowAbove + sampleColLeft]->getDepth() >= 0.0f)
            {
                votes = 7;
            }
            if (biomes.data[sampleRowAbove + sampleColRight]->getDepth() >= 0.0f)
            {
                --votes;
            }
            if (biomes.data[sampleRowAbove + sampleColMid]->getDepth() >= 0.0f)
            {
                --votes;
            }
            if (biomes.data[sampleRowBelow + sampleColLeft]->getDepth() >= 0.0f)
            {
                --votes;
            }
            if (biomes.data[sampleRowBelow + sampleColRight]->getDepth() >= 0.0f)
            {
                --votes;
            }
            if (biomes.data[sampleRowBelow + sampleColMid]->getDepth() >= 0.0f)
            {
                --votes;
            }
            if (biomes.data[sampleRow + sampleColLeft]->getDepth() >= 0.0f)
            {
                --votes;
            }
            if (biomes.data[sampleRow + sampleColRight]->getDepth() >= 0.0f)
            {
                --votes;
            }

            const MaterialColor* color;
            int shade;

            if (center->getDepth() >= 0.0f)
            {
                if (votes < 4)
                {
                    continue;
                }
                color = MaterialColor::COLOR_BROWN;
                shade = 1;
            }
            else
            {
                color = MaterialColor::COLOR_ORANGE;
                shade = 0;
                if (votes == 8)
                {
                    if ((mapZ & 1) != 0)
                    {
                        continue;
                    }
                    int hashOffset = (int)(Mth::sin((float)mapZ) * 32768.013729155304);
                    shade = ((mapX + hashOffset) / 8) % 5;
                    if (shade == 3)
                    {
                        color = MaterialColor::COLOR_BROWN;
                        shade = 1;
                    }
                    else if (shade == 4)
                    {
                        shade = 0;
                    }
                }
                else if (votes > 5)
                {
                    color = MaterialColor::COLOR_BROWN;
                    shade = 1;
                }
                else if (votes < 2)
                {
                    shade = 3;
                }
            }

            int index = mapX + mapZ * 128;
            if (data->mColors.data[index] == 0)
            {
                data->mColors.data[index] = static_cast<unsigned char>(color->id * 4 + shade);
                data->setDirty(mapX, mapZ);
            }
        }
    }
}

void MapItem::inventoryTick(not_null_ptr<ItemInstance> itemInstance, Level* level,
                            boost::shared_ptr<Entity> entity, int slot, bool selected)
{
    if (level->mIsClientSide)
    {
        return;
    }

    boost::shared_ptr<MapItemSavedData> data = getSavedData(itemInstance, level);

    if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        int newMapId = level->getAuxValueForMap(player->mXuid, data->mDimension, data->mXCenter,
                                                data->mZCenter, data->mScale);
        if (newMapId == itemInstance->getAuxValue())
        {
            data->tickCarriedBy(player, itemInstance);
        }
        else
        {
            boost::shared_ptr<MapItemSavedData> newData = getSavedData(newMapId, level);
            newData->mXCenter = data->mXCenter;
            newData->mZCenter = data->mZCenter;
            newData->mScale = data->mScale;
            newData->mDimension = data->mDimension;
            newData->mTrackingPosition = newData->mTrackingPosition || data->mTrackingPosition;
            itemInstance->setAuxValue(newMapId);
            newData->tickCarriedBy(player, itemInstance);
            newData->mergeInMapData(data);
            player->mInventoryMenu->broadcastChanges();
            data = newData;
        }
    }

    bool shouldUpdate;
    if (selected)
    {
        shouldUpdate = true;
    }
    else if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        shouldUpdate = (player->getOffhandItem() == itemInstance);
    }
    else
    {
        shouldUpdate = false;
    }

    if (shouldUpdate)
    {
        update(level, entity, data);
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
    itemInstance->setAuxValue(level->getFreeAuxValueFor(L"map"));
    std::wstring name = L"map_" + _toString(itemInstance->getAuxValue());
    boost::shared_ptr<MapItemSavedData> data(new MapItemSavedData(name));
    if (oldData)
    {
        data->mScale = Mth::clamp(oldData->mScale + scale, 0, 4);
        data->mTrackingPosition = oldData->mTrackingPosition;
        data->setOrigin(static_cast<double>(oldData->mXCenter), static_cast<double>(oldData->mZCenter),
                        data->mScale);
        data->mDimension = oldData->mDimension;
        data->setDirty();
        level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
    }
}

void MapItem::enableMapTracking(not_null_ptr<ItemInstance> itemInstance, Level* level)
{
    boost::shared_ptr<MapItemSavedData> oldData =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(itemInstance, level);
    itemInstance->setAuxValue(level->getFreeAuxValueFor(L"map"));
    std::wstring name = L"map_" + _toString(itemInstance->getAuxValue());
    boost::shared_ptr<MapItemSavedData> data(new MapItemSavedData(name));
    if (oldData)
    {
        data->mTrackingPosition = true;
        data->mXCenter = oldData->mXCenter;
        data->mZCenter = oldData->mZCenter;
        data->mScale = oldData->mScale;
        data->mDimension = oldData->mDimension;
        data->setDirty();
        level->setSavedData(name, boost::static_pointer_cast<SavedData>(data));
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
    if (tag->contains(L"map_scale_direction", Tag::TAG_ANY_NUMERIC))
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
    if (!display || !display->contains(L"MapColor", Tag::TAG_ANY_NUMERIC))
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
