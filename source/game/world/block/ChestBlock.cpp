#include "game/world/block/ChestBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Container.hpp"
#include "game/item/Containers.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"

ChestBlock::ChestBlock(Type type)
    : BaseEntityBlock(NULL), mType(type)
{
    mBlockEntity = true;
    DerivedInit();
    registerDefaultState(mStateDefinition->any());
}

ChestBlock::~ChestBlock()
{
}

boost::shared_ptr<BlockEntity> ChestBlock::newBlockEntity(Level* level, int data)
{
    return boost::shared_ptr<BlockEntity>();
}

void ChestBlock::blockStaticCtor()
{
}

bool ChestBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

bool ChestBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool ChestBlock::hasCustomBreakingProgress(const BlockState* state)
{
    return true;
}

const AABB* ChestBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

void ChestBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
}

const BlockState* ChestBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                    float hitX, float hitY, float hitZ, int auxValue,
                                                    boost::shared_ptr<LivingEntity> placer)
{
    return getBlockState(auxValue);
}

const BlockState* ChestBlock::recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state)
{
    return state;
}

void ChestBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
}

const BlockState* ChestBlock::reorient(Level* level, const BlockPos& pos, const BlockState* state)
{
    return state;
}

bool ChestBlock::isFullChest(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    return state != NULL && state->getBlock() == this && getContainer(level, pos).get() != NULL;
}

bool ChestBlock::mayPlace(Level* level, const BlockPos& pos)
{
    int count = 0;
    BlockPos positions[4] = { pos.west(), pos.east(), pos.north(), pos.south() };
    for (int i = 0; i < 4; ++i)
    {
        if (level->getBlockState(positions[i]) != NULL && level->getBlockState(positions[i])->getBlock() == this)
        {
            if (isFullChest(level, positions[i]))
                return false;
            ++count;
        }
    }
    return count < 2;
}

void ChestBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                 Block* block, const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
}

void ChestBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (entity.get() != NULL)
    {
        boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(entity);
        if (container.get() != NULL)
            Containers::dropContents(level, pos, container);
        level->blockEntityChanged(pos, entity);
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool ChestBlock::TestUse()
{
    return true;
}

bool ChestBlock::isBlockedChestByBlock(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.above()) != NULL && level->getBlockState(pos.above())->getBlock()->isSolidRender(level->getBlockState(pos.above()));
}

bool ChestBlock::isEntityOnChest(Level* level, const BlockPos& pos)
{
    return false;
}

bool ChestBlock::isChestBlockedAt(Level* level, const BlockPos& pos)
{
    return isBlockedChestByBlock(level, pos) || isEntityOnChest(level, pos);
}

boost::shared_ptr<Container> ChestBlock::getContainer(Level* level, const BlockPos& pos, bool force)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    return boost::dynamic_pointer_cast<Container>(entity);
}

boost::shared_ptr<Container> ChestBlock::getContainer(Level* level, const BlockPos& pos)
{
    return getContainer(level, pos, false);
}

bool ChestBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                     boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                     const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    if (clientSide || isChestBlockedAt(level, pos))
        return true;
    boost::shared_ptr<Container> container = getContainer(level, pos);
    if (container.get() == NULL)
        return true;
    return true;
}

bool ChestBlock::isSignalSource(const BlockState* state)
{
    return true;
}

int ChestBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction)
{
    return 0;
}

int ChestBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction)
{
    return getSignal(state, levelSource, pos, direction);
}

bool ChestBlock::hasAnalogOutputSignal(const BlockState* state)
{
    return true;
}

int ChestBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<Container> container = getContainer(level, pos);
    return container.get() == NULL ? 0 : AbstractContainerMenu::getRedstoneSignalFromContainer(container);
}

const BlockState* ChestBlock::getBlockState(int data)
{
    return defaultBlockState();
}

int ChestBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return 0;
}

unsigned int ChestBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* ChestBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state;
}

const BlockState* ChestBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state;
}

BlockStateDefinition* ChestBlock::createBlockStateDefinition()
{
    return new BlockStateDefinition(this);
}

EBlockFaceShape ChestBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                              const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

void ChestBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"chest_top");
}

EBlockRenderShape ChestBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_ENTITY;
}

EPistonPushReaction ChestBlock::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_DESTROY;
}
