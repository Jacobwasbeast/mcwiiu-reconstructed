#include "game/world/block/BlockEntity.hpp"

#include <map>
#include <boost/unordered_map.hpp>

#include "game/world/block/Block.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/world/block/JukeboxBlock.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/client/audio/MusicBlockEntity.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/DaylightDetectorBlockEntity.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/block/ComparatorBlockEntity.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/world/block/CommandBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"

namespace
{
    typedef boost::unordered_map<std::wstring, BlockEntity::Creator> CreatorMap;
    typedef MappedRegistry<ResourceLocation, unsigned long long> TypeRegistry;
    CreatorMap gCreators;
    TypeRegistry gTypes;
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

CompoundTag* BlockEntity::save(CompoundTag* tag)
{
    return saveMetadata(tag);
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
        mLevel->blockEntityChanged(mPos, shared_from_this());
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
    Register(1, FurnaceBlockEntity::create, 0x1e00202ULL, L"furnace");
    Register(2, ChestBlockEntity::create, 0x1e00201ULL, L"chest");
    Register(3, EnderChestBlockEntity::create, 0x1200208ULL, L"ender_Chest");
    Register(4, JukeboxBlock::Entity::create, 0x1000204ULL, L"jukebox");
    Register(5, DispenserBlockEntity::create, 0x3c00200ULL, L"dispenser");
    Register(6, DropperBlockEntity::create, 0x3c00201ULL, L"dropper");
    Register(7, SignBlockEntity::create, 0x1000202ULL, L"sign");
    Register(8, MobSpawnerBlockEntity::create, 0x1200201ULL, L"mob_spawner");
    Register(9, MusicBlockEntity::create, 0x1000203ULL, L"noteblock");
    Register(10, PistonMovingBlockEntity::create, 0x1200205ULL, L"piston");
    Register(11, BrewingStandBlockEntity::create, 0x1e00203ULL, L"brewing_stand");
    Register(12, EnchantmentTableEntity::create, 0x1200206ULL, L"enchanting_table");
    Register(13, TheEndPortalBlockEntity::create, 0x1300200ULL, L"end_portal");
    Register(14, BeaconBlockEntity::create, 0x1e00204ULL, L"beacon");
    Register(15, SkullBlockEntity::create, 0x1200207ULL, L"skull");
    Register(16, DaylightDetectorBlockEntity::create, 0x120020bULL, L"daylight_detector");
    Register(17, HopperBlockEntity::create, 0x1e00205ULL, L"hopper");
    Register(18, ComparatorBlockEntity::create, 0x100020aULL, L"comparator");
    Register(19, FlowerPotBlockEntity::create, 0x100020dULL, L"flower_pot");
    Register(20, BannerBlockEntity::create, 0x100020cULL, L"banner");
    Register(22, TheEndGatewayEntity::create, 0x1300201ULL, L"end_gateway");
    Register(23, CommandBlockEntity::create, 0x1000209ULL, L"command_block");
    Register(24, ShulkerBoxBlockEntity::create, 0x120020fULL, L"shulker_box");
    Register(25, BedBlockEntity::create, 0x1000210ULL, L"bed");
    Register(26, CauldronBlockEntity::create, 0x1000211ULL, L"cauldron");
    Register(27, ConduitBlockEntity::create, 0x1200212ULL, L"conduit");
}

void BlockEntity::Register(int id, Creator creator, unsigned long long type, const std::wstring& name)
{
    gCreators[name] = creator;
    gTypes.registerMapping(id, ResourceLocation(name), type);
}

ResourceLocation BlockEntity::getKey(unsigned long long type)
{
    return gTypes.getKey(type);
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
