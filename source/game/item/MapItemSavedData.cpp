#include "game/item/MapItemSavedData.hpp"
#include <algorithm>
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Math.hpp"
#include "game/world/block/BlockPos.hpp"

MapItemSavedData::MapItemSavedData(const std::wstring& id)
    : SavedData(id), mXCenter(0), mZCenter(0), mDimension(0), mTrackingPosition(true),
      mUnlimitedTracking(0), mScale(0), mColors(0x4000, true)
{
}

MapItemSavedData::~MapItemSavedData()
{
}

void MapItemSavedData::setOrigin(double x, double z, int scale)
{
    int size = (1 << (scale & 0x3f)) * 0x80;
    mXCenter = (int)Math::round(x / (double)size) * size;
    mZCenter = (int)Math::round(z / (double)size) * size;
}

boost::shared_ptr<MapItemSavedData::HoldingPlayer> MapItemSavedData::getHoldingPlayer(boost::shared_ptr<Player> player)
{
    boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, PlayerKeyHash, PlayerKeyEq>::iterator it = mCarriedBy.find(player);
    if (it != mCarriedBy.end()) return it->second;
    boost::shared_ptr<HoldingPlayer> holding(new HoldingPlayer(player, this));
    mCarriedBy[player] = holding;
    mHoldingPlayers.push_back(holding);
    return holding;
}

void MapItemSavedData::setDirty(int x, int y)
{
    SavedData::setDirty();
    std::vector<boost::shared_ptr<HoldingPlayer> >::iterator it;
    for (it = mHoldingPlayers.begin(); it != mHoldingPlayers.end(); ++it) (*it)->markDirty(x, y);
}

int MapItemSavedData::fjEnumerateAxisPoint(int point)
{
    int value = point / (1 << (mScale & 0x3f)) + 0x40;
    return value - (((value >> 31) & 0x7f) + value & ~0x7f);
}

int MapItemSavedData::fjGetDecorationId(CompoundTag* tag)
{
    int x = (int)tag->getDouble(L"x") - mXCenter;
    int z = (int)tag->getDouble(L"z") - mZCenter;
    return -(fjEnumerateAxisPoint(x) * 0x80 + fjEnumerateAxisPoint(z) + tag->getByte(L"type"));
}

void MapItemSavedData::addDecoration(const MapDecoration::Type* type, Level*, int id, double x, double z, double rotation, double, bool visible)
{
    float scale = (float)(1 << (mScale & 0x3f));
    float mapX = (float)((x - mXCenter) / scale);
    float mapZ = (float)((z - mZCenter) / scale);
    int imageX = (int)(mapX * 2.0f + 0.5f);
    int imageZ = (int)(mapZ * 2.0f + 0.5f);
    int rot = (int)(((rotation + (rotation < 0.0 ? -8.0 : 8.0)) * 16.0) / 360.0);
    boost::unordered_map<int, MapDecoration>* decorations = type->isPlayer() ? &mDecorations : &mDecorationsById;
    if (!type->isPlayer() && (mapX < -63.0f || mapX >= 63.0f || mapZ < -63.0f || mapZ >= 63.0f)) { decorations->erase(id); return; }
    if (mapX < -64.0f) imageX = -128;
    if (mapZ < -64.0f) imageZ = -128;
    if (mapX >= 64.0f) imageX = 127;
    if (mapZ >= 64.0f) imageZ = 127;
    (*decorations)[id] = MapDecoration(type, (char)imageX, (char)imageZ, (char)rot, id, visible);
}

void MapItemSavedData::tickCarriedBy(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance>)
{
    getHoldingPlayer(player)->markDirty(0, 0);
}

void MapItemSavedData::mergeInMapData(boost::shared_ptr<MapItemSavedData> data)
{
    int x;
    int y;
    for (y = 0; y < 128; ++y) for (x = 0; x < 128; ++x)
    {
        int index = y * 128 + x;
        if (mColors.data[index] == 0 && data->mColors.data[index] != 0) mColors.data[index] = data->mColors.data[index];
        setDirty(x, y);
    }
}

std::vector<MapDecoration> MapItemSavedData::fjGetAllDecorations()
{
    std::vector<MapDecoration> result;
    boost::unordered_map<int, MapDecoration>::const_iterator it;
    for (it = mDecorations.begin(); it != mDecorations.end(); ++it) result.push_back(it->second);
    for (it = mDecorationsById.begin(); it != mDecorationsById.end(); ++it) result.push_back(it->second);
    return result;
}

boost::shared_ptr<ClientboundMapItemDataPacket> MapItemSavedData::getUpdatePacket(not_null_ptr<ItemInstance> item, Level*, boost::shared_ptr<Player> player)
{
    boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, PlayerKeyHash, PlayerKeyEq>::iterator it = mCarriedBy.find(player);
    if (it == mCarriedBy.end()) return boost::shared_ptr<ClientboundMapItemDataPacket>();
    return it->second->nextUpdatePacket(item);
}

void MapItemSavedData::load(CompoundTag* tag)
{
    mDimension = tag->getByte(L"dimension");
    mXCenter = tag->getInt(L"xCenter");
    mZCenter = tag->getInt(L"zCenter");
    mScale = (unsigned char)std::max(0, std::min(4, (int)tag->getByte(L"scale")));
    mTrackingPosition = !tag->contains(L"trackingPosition", 1) || tag->getBoolean(L"trackingPosition");
    mUnlimitedTracking = tag->getBoolean(L"unlimitedTracking");
    int width = tag->getShort(L"width");
    int height = tag->getShort(L"height");
    arrayWithLength<unsigned char> colors = tag->getByteArray(L"colors");
    if (width == 128 && height == 128)
    {
        for (unsigned int i = 0; i < colors.length && i < mColors.length; ++i) mColors.data[i] = colors.data[i];
        return;
    }
    for (unsigned int i = 0; i < mColors.length; ++i) mColors.data[i] = 0;
    int xOffset = (128 - width) / 2;
    int yOffset = (128 - height) / 2;
    int x;
    int y;
    for (y = 0; y < height; ++y) for (x = 0; x < width; ++x)
    {
        int dstX = x + xOffset;
        int dstY = y + yOffset;
        if (dstX >= 0 && dstX < 128 && dstY >= 0 && dstY < 128 && (unsigned int)(y * width + x) < colors.length) mColors.data[dstY * 128 + dstX] = colors.data[y * width + x];
    }
}

CompoundTag* MapItemSavedData::save(CompoundTag* tag)
{
    tag->putByte(L"dimension", mDimension);
    tag->putInt(L"xCenter", mXCenter);
    tag->putInt(L"zCenter", mZCenter);
    tag->putByte(L"scale", mScale);
    tag->putShort(L"width", 128);
    tag->putShort(L"height", 128);
    tag->putByteArray(L"colors", mColors);
    tag->putBoolean(L"trackingPosition", mTrackingPosition);
    tag->putBoolean(L"unlimitedTracking", mUnlimitedTracking != 0);
    return tag;
}

void MapItemSavedData::addTargetDecoration(const not_null_ptr<ItemInstance>& item, const BlockPos& pos, const std::wstring& id, const MapDecoration::Type* type)
{
    ListTag* decorations;
    if (item->hasTag() && item->getTag()->contains(L"Decorations", 9)) decorations = item->getTag()->getList(L"Decorations", 10);
    else { decorations = new ListTag(10); item->addTagElement(L"Decorations", decorations); }
    CompoundTag* decoration = new CompoundTag();
    decoration->putByte(L"type", type->getIcon());
    decoration->putString(L"id", id);
    decoration->putDouble(L"x", (double)pos.x);
    decoration->putDouble(L"z", (double)pos.z);
    decoration->putDouble(L"rot", 180.0);
    decorations->add(decoration);
    if (type->hasMapColor())
    {
        CompoundTag* display = static_cast<CompoundTag*>(item->getOrCreateTagElement(L"display"));
        display->putInt(L"MapColor", type->getMapColor());
    }
}

bool MapItemSavedData::fjIsCompletelyExplored() const
{
    int unexplored = 0;
    for (unsigned int i = 0; i < mColors.length; ++i) if (mColors.data[i] == 0) ++unexplored;
    return unexplored < 0x333;
}

void MapItemSavedData::fjRemoveDecorationByEntityId(int id)
{
    mDecorationsById.erase(id);
}

MapItemSavedData::HoldingPlayer::HoldingPlayer(boost::shared_ptr<Player> player, MapItemSavedData* data)
    : mPlayer(player), mDirty(false), mMinX(0), mMinY(0), mMaxX(127), mMaxY(127), mTick(0), mMapData(data)
{
}

MapItemSavedData::HoldingPlayer::~HoldingPlayer()
{
}

void MapItemSavedData::HoldingPlayer::markDirty(int x, int y)
{
    if (mDirty)
    {
        mMinX = std::min(mMinX, x);
        mMinY = std::min(mMinY, y);
        mMaxX = std::max(mMaxX, x);
        mMaxY = std::max(mMaxY, y);
    }
    else
    {
        mDirty = true;
        mMinX = x;
        mMinY = y;
        mMaxX = x;
        mMaxY = y;
    }
}

boost::shared_ptr<ClientboundMapItemDataPacket> MapItemSavedData::HoldingPlayer::nextUpdatePacket(not_null_ptr<ItemInstance>)
{
    ++mTick;
    if (!mDirty || mMapData == NULL) return boost::shared_ptr<ClientboundMapItemDataPacket>();
    mDirty = false;
    return boost::shared_ptr<ClientboundMapItemDataPacket>();
}
