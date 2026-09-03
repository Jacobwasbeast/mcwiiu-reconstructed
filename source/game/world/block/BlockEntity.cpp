#include "game/world/block/BlockEntity.hpp"

#include <map>

#include "game/world/block/Block.hpp"
#include "game/world/block/MappedRegistry.hpp"

namespace
{
    typedef std::map<std::wstring, BlockEntity::Creator> CreatorMap;
    typedef std::map<unsigned long long, ResourceLocation> KeyMap;
    CreatorMap gCreators;
    KeyMap gKeys;
}

BlockEntity::BlockEntity()
    : mRemoved(false), mRenderRemoveStage(0), mLevel(NULL), mPos(), mData(-1), mBlock(NULL)
{
}

BlockEntity::~BlockEntity()
{
}

void BlockEntity::rotate(Rotation* rotation)
{
}

void BlockEntity::mirror(Mirror* mirror)
{
}

void BlockEntity::setRenderRemoveStage(unsigned char stage)
{
    mRenderRemoveStage = stage;
}

void BlockEntity::upgradeRenderRemoveStage()
{
    if (mRenderRemoveStage == 1)
    {
        mRenderRemoveStage = 2;
    }
}

bool BlockEntity::shouldRemoveForRender()
{
    return mRenderRemoveStage == 2;
}

Level* BlockEntity::getLevel()
{
    return mLevel;
}

void BlockEntity::setLevel(Level* level)
{
    mLevel = level;
}

bool BlockEntity::hasLevel()
{
    return mLevel != NULL;
}

void BlockEntity::load(CompoundTag* tag)
{
    mPos = BlockPos(tag->getInt(L"x"), tag->getInt(L"y"), tag->getInt(L"z"));
}

CompoundTag* BlockEntity::saveMetadata(CompoundTag* tag)
{
    ResourceLocation key = getKey(GetType());
    tag->putString(L"id", key.toString());
    tag->putInt(L"x", mPos.x);
    tag->putInt(L"y", mPos.y);
    tag->putInt(L"z", mPos.z);
    return tag;
}

void BlockEntity::save(CompoundTag* tag)
{
    saveMetadata(tag);
}

void BlockEntity::tick()
{
}

void BlockEntity::setLevelForLoading(Level* level)
{
    mLevel = level;
}

int BlockEntity::getData()
{
    if (mData == -1 && mLevel != NULL)
    {
        mData = getBlock() == NULL ? 0 : 0;
    }
    return mData;
}

void BlockEntity::setData(int data)
{
    mData = data;
}

void BlockEntity::setChanged()
{
    if (mLevel != NULL)
    {
        mData = getData();
    }
}

double BlockEntity::distanceToSqr(double x, double y, double z)
{
    const double dx = (double)mPos.x + 0.5 - x;
    const double dy = (double)mPos.y + 0.5 - y;
    const double dz = (double)mPos.z + 0.5 - z;
    return dx * dx + dy * dy + dz * dz;
}

double BlockEntity::getViewDistance()
{
    return 4096.0;
}

BlockPos BlockEntity::getBlockPos()
{
    return mPos;
}

Block* BlockEntity::getBlock()
{
    return mBlock;
}

boost::shared_ptr<Packet> BlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<Packet>();
}

CompoundTag* BlockEntity::getUpdateTag()
{
    CompoundTag* tag = new CompoundTag();
    return saveMetadata(tag);
}

CompoundTag* BlockEntity::getUpdateTagForPistonMovement()
{
    return getUpdateTag();
}

bool BlockEntity::isRemoved()
{
    return mRemoved;
}

void BlockEntity::setRemoved()
{
    mRemoved = true;
}

void BlockEntity::clearRemoved()
{
    mRemoved = false;
}

bool BlockEntity::triggerEvent(int id, int param)
{
    return false;
}

void BlockEntity::clearCache()
{
    mBlock = NULL;
    mData = -1;
}

void BlockEntity::setPosition(const BlockPos& position)
{
    mPos = position;
}

bool BlockEntity::onlyOpCanSetNbt()
{
    return false;
}

std::wstring BlockEntity::getDisplayName()
{
    return std::wstring();
}

void BlockEntity::clone(boost::shared_ptr<BlockEntity> other)
{
    if (!other)
    {
        return;
    }
    other->mLevel = mLevel;
    other->mPos = mPos;
    other->mData = mData;
    other->mBlock = mBlock;
}

void BlockEntity::operator=(const BlockEntity& other)
{
    mRemoved = other.mRemoved;
    mRenderRemoveStage = other.mRenderRemoveStage;
    mLevel = other.mLevel;
    mPos = other.mPos;
    mData = other.mData;
    mBlock = other.mBlock;
}

void BlockEntity::staticCtor()
{
}

void BlockEntity::Register(int id, Creator creator, unsigned long long type, const std::wstring& name)
{
    gCreators[name] = creator;
    gKeys[type] = ResourceLocation(name);
}

ResourceLocation BlockEntity::getKey(unsigned long long type)
{
    KeyMap::const_iterator it = gKeys.find(type);
    if (it == gKeys.end())
    {
        return ResourceLocation();
    }
    return it->second;
}

boost::shared_ptr<BlockEntity> BlockEntity::loadStatic(Level* level, CompoundTag* tag)
{
    const std::wstring name = tag->getString(L"id");
    CreatorMap::const_iterator it = gCreators.find(name);
    if (it == gCreators.end() || it->second == NULL)
    {
        return boost::shared_ptr<BlockEntity>();
    }
    boost::shared_ptr<BlockEntity> entity(it->second());
    if (entity)
    {
        entity->setLevelForLoading(level);
        entity->load(tag);
    }
    return entity;
}
