#include "game/world/block/BedBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/block/MaterialColor.hpp"

BedBlock::Part::Part(int index, const std::wstring& name)
    : mName(name), mIndex(index)
{
}

std::wstring BedBlock::Part::toString() const
{
    return mName;
}

std::wstring BedBlock::Part::getSerializedName() const
{
    return mName;
}

BedBlock::BedBlock()
    : HorizontalDirectionalBlock(NULL), EntityBlock()
{
    mBlockEntity = true;
    DerivedInit();
    registerDefaultState(mStateDefinition->any());
}

BedBlock::~BedBlock()
{
}

boost::shared_ptr<BlockEntity> BedBlock::newBlockEntity(Level* level, int data)
{
    return boost::shared_ptr<BlockEntity>(new BedBlockEntity());
}

void BedBlock::blockStaticCtor()
{
    Part first(0, L"foot");
    Part second(1, L"head");
}

const MaterialColor* BedBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::getDyeMaterial(DyeColor::WHITE);
}

bool BedBlock::TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    return false;
}

boost::shared_ptr<Player> BedBlock::getSleepingPlayerAt(Level* level, const BlockPos& pos)
{
    return boost::shared_ptr<Player>();
}

bool BedBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    return TestUse(level, pos, state, player);
}

bool BedBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool BedBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

void BedBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    Block::fallOn(level, pos, entity, fallDistance);
}

void BedBlock::updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity)
{
    Block::updateEntityAfterFallOn(level, entity);
}

void BedBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
}

Item* BedBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Block::getResource(state, random, lootBonus);
}

const AABB* BedBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool BedBlock::hasCustomBreakingProgress(const BlockState* state)
{
    return true;
}

bool BedBlock::BlockIsDoor(const Block* block)
{
    return block == NULL;
}

bool BedBlock::hasSolidGroundAndTwoBlocksSpaceAt(Level* level, const BlockPos& pos)
{
    return false;
}

void BedBlock::findStandUpPosition(Level* level, const BlockPos& pos, int data)
{
    if (level != NULL && data >= 0)
    {
        hasSolidGroundAndTwoBlocksSpaceAt(level, pos);
    }
}

void BedBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);
}

EPistonPushReaction BedBlock::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_DESTROY;
}

int BedBlock::getRenderLayer()
{
    return 0;
}

EBlockRenderShape BedBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_ENTITY;
}

not_null_ptr<ItemInstance> BedBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return not_null_ptr<ItemInstance>();
}

bool BedBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player)
{
    return Block::playerWillDestroy(level, pos, state, player);
}

void BedBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos, const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity, not_null_ptr<ItemInstance> tool)
{
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

void BedBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
}

const BlockState* BedBlock::getBlockState(int data)
{
    return Block::getBlockState(data);
}

const BlockState* BedBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state;
}

int BedBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return Block::convertBlockStateToLegacyData(state);
}

EBlockFaceShape BedBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

unsigned int BedBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return 0;
}

const BlockState* BedBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return Block::rotate(state, rotation);
}

const BlockState* BedBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return Block::mirror(state, mirror);
}

BlockStateDefinition* BedBlock::createBlockStateDefinition()
{
    return new BlockStateDefinition(this);
}

bool BedBlock::isHeadPiece(int data)
{
    return (data & 8) != 0;
}

void BedBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
}
