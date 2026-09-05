#include "game/item/MapItem.hpp"

#include <string>
#include <typeinfo>

#include "game/core/IconRegister.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"

std::wstring _toString(int value);

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

void MapItem::update(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<MapItemSavedData> data)
{
    if (data)
    {
        data->setDirty();
    }
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
