#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/BlockId.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PistonMagicBlock.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/generation/PistonStructureResolver.hpp"
#include "game/entity/mob/LivingEntity.hpp"

AbstractProperty<const Direction*>* PistonBaseBlock::FACING = NULL;
AbstractProperty<bool>* PistonBaseBlock::EXTENDED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"extended"));
static const AABB* PISTON_EXTENDED_DOWN = AABB::newPermanent(0.0, 0.25, 0.0, 1.0, 1.0, 1.0);
static const AABB* PISTON_EXTENDED_UP = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0);
static const AABB* PISTON_EXTENDED_NORTH = AABB::newPermanent(0.0, 0.0, 0.25, 1.0, 1.0, 1.0);
static const AABB* PISTON_EXTENDED_SOUTH = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.75);
static const AABB* PISTON_EXTENDED_WEST = AABB::newPermanent(0.25, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* PISTON_EXTENDED_EAST = AABB::newPermanent(0.0, 0.0, 0.0, 0.75, 1.0, 1.0);

PistonBaseBlock::PistonBaseBlock(bool sticky)
    : DirectionalBlock(Material::PISTON), mSticky(sticky), mInnerTopIcon(NULL), mBottomIcon(NULL), mTopIcon(NULL)
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::UP);
    TypedBoxed<bool> extended(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(FACING, &facing)->setBoxedValue(EXTENDED, &extended));
    setSoundType(SoundType::STONE);
    setDestroyTime(0.5f);
}

PistonBaseBlock::~PistonBaseBlock()
{
}

void PistonBaseBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

bool PistonBaseBlock::isViewBlocking(const BlockState* state)
{
    return !state->getValue(EXTENDED);
}

int PistonBaseBlock::getSpawnResourcesAuxValue(const BlockState*) { return 5; }

const AABB* PistonBaseBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (!state->getValue(EXTENDED))
        return Block::getShape(state, levelSource, pos);

    switch (state->getValue(FACING)->get3DDataValue())
    {
    case Direction::eDirection_Down: return PISTON_EXTENDED_DOWN;
    case Direction::eDirection_Up: return PISTON_EXTENDED_UP;
    case Direction::eDirection_North: return PISTON_EXTENDED_NORTH;
    case Direction::eDirection_South: return PISTON_EXTENDED_SOUTH;
    case Direction::eDirection_West: return PISTON_EXTENDED_WEST;
    case Direction::eDirection_East: return PISTON_EXTENDED_EAST;
    default: return PISTON_EXTENDED_UP;
    }
}

bool PistonBaseBlock::isTopSolidBlocking(const BlockState* state)
{
    return !state->getValue(EXTENDED) && state->getValue(FACING) == Direction::DOWN;
}

void PistonBaseBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                        const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                        boost::shared_ptr<Entity> entity, bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, state->getShape(level, pos));
}

bool PistonBaseBlock::isSolidRender(const BlockState*) const { return false; }

int PistonBaseBlock::getNeighborSignal(Level* level, const BlockPos& pos, const Direction* direction)
{
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* side = *it;
        if (side != direction && level->hasSignal(pos.relative(side), side))
            return true;
    }

    if (level->hasSignal(pos, Direction::DOWN))
        return true;

    BlockPos above = pos.above();
    for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        const Direction* side = *it;
        if (side != Direction::DOWN && level->hasSignal(above.relative(side), side))
        {
            const BlockState* neighbourState = level->getBlockState(above.relative(side));
            Block* neighbour = level->getBlock(above.relative(side));
            if (!neighbour->mBlockEntity)
                return true;
            return neighbourState->getPistonPushReaction() != PISTON_PUSH_NORMAL;
        }
    }
    return false;
}

bool PistonBaseBlock::IsPushRestrictedByLevelHeight(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos target = pos.relative(direction);
    return level->getMaxBuildHeight() <= target.y;
}

int PistonBaseBlock::isPushable(const BlockState* state, Level* level, const BlockPos& pos, const Direction* direction,
                                bool allowDestroy, const Direction* motionDirection)
{
    Block* block = state->getBlock();
    if (block == Blocks::obsidian)
        return false;
    if (!level->getWorldBorder()->isWithinBounds(pos) || pos.y < 0 || (direction == Direction::DOWN && pos.y == 0)
        || IsPushRestrictedByLevelHeight(level, pos, direction))
        return false;

    int minXZ = -(level->mDimension->getXZSize() * 16) >> 1;
    int maxXZ = ((level->mDimension->getXZSize() << 4) >> 1) - 1;
    if (pos.x <= minXZ || pos.x >= maxXZ || pos.z <= minXZ || pos.z >= maxXZ)
        return false;

    if (block == Blocks::piston || block == Blocks::sticky_piston)
    {
        if (!level->getBlockState(pos)->getValue(EXTENDED))
            return true;
    }
    else
    {
        if (Block::getId(block) == BlockId::CHEST || block == Blocks::trapped_chest)
            return block->mayPlace(level, pos.relative(direction));
        if (state->getDestroySpeed(level, pos) != Block::INDESTRUCTIBLE)
        {
            switch (state->getPistonPushReaction())
            {
            case PISTON_PUSH_DESTROY:
                return allowDestroy;
            case PISTON_PUSH_BLOCK:
                return false;
            case PISTON_PUSH_PUSH_ONLY:
                return direction == motionDirection;
            default:
                return true;
            }
        }
    }
    return false;
}

void PistonBaseBlock::checkIfExtend(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (getNeighborSignal(level, pos, facing))
    {
        if (!state->getValue(EXTENDED))
        {
            PistonStructureResolver resolver(level, pos, facing, true);
            if (resolver.resolve())
                level->blockEvent(pos, this, 0, facing->get3DDataValue());
        }
    }
    else if (state->getValue(EXTENDED))
    {
        level->blockEvent(pos, this, 1, facing->get3DDataValue());
    }
}

void PistonBaseBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance>)
{
    TypedBoxed<const Direction*> facing(Direction::getDirectionFromLivingEntity(pos, placer));
    level->setBlock(pos, state->setBoxedValue(FACING, &facing), Block::UPDATE_CLIENTS, false);
    if (!level->mIsClientSide)
        checkIfExtend(level, pos, state);
}

void PistonBaseBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                      Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
        checkIfExtend(level, pos, state);
}

void PistonBaseBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide && level->getBlockEntity(pos) == NULL)
        checkIfExtend(level, pos, state);
}

const Direction* PistonBaseBlock::getFacingFromPlayer(Level*, const BlockPos& pos, boost::shared_ptr<LivingEntity> placer)
{
    float xDistance = Mth::abs(static_cast<float>(placer->mX) - (static_cast<float>(pos.x) + 0.5f));
    if (xDistance < 2.5)
    {
        float zDistance = Mth::abs(static_cast<float>(placer->mZ) - (static_cast<float>(pos.z) + 0.5f));
        if (zDistance < 2.5)
        {
            double eyeY = placer->mY + placer->getEyeHeight();
            double yDistance = eyeY - static_cast<double>(pos.y);
            if (yDistance > 2.0)
                return Direction::UP;
            if (yDistance < 0.0)
                return Direction::DOWN;
        }
    }
    return placer->getDirection()->getOpposite();
}

const BlockState* PistonBaseBlock::getStateForPlacement(Level* level, const BlockPos& pos,
                                                         const Direction*, float, float, float, int,
                                                         boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = getFacingFromPlayer(level, pos, placer);
    TypedBoxed<const Direction*> boxed(facing);
    return defaultBlockState()->setBoxedValue(FACING, &boxed);
}

bool PistonBaseBlock::canContainLiquid(const BlockState*) const { return true; }

int PistonBaseBlock::moveBlocks(Level* level, const BlockPos& pos, const Direction* direction, bool extending)
{
    if (!extending)
        level->removeBlock(pos.relative(direction));

    PistonStructureResolver resolver(level, pos, direction, extending);
    if (!resolver.resolve())
        return false;

    BlockPos headPos = pos.relative(direction);
    bool headInWater = true;
    if (level->getLiquidBlockState(headPos)->getMaterial() != Material::WATER
        && level->getLiquidBlockState(headPos.above())->getMaterial() != Material::WATER)
    {
        headInWater = false;
    }

    std::vector<BlockPos>& toPush = resolver.getToPush();
    std::vector<const BlockState*> pushStates;
    for (unsigned int i = 0; i < toPush.size(); ++i)
    {
        const BlockPos& pushPos = toPush.at(i);
        pushStates.push_back(level->getBlockState(pushPos)->fillVirtualBlockStateProperties(level, pushPos));
    }

    std::vector<BlockPos>& toDestroy = resolver.getToDestroy();
    int remaining = toPush.size() + toDestroy.size();
    const BlockState** movedStates = new const BlockState*[remaining];
    const Direction* moveDirection = extending ? direction : direction->getOpposite();

    for (int i = toDestroy.size() - 1; i >= 0; --i)
    {
        BlockPos destroyPos = toDestroy.at(i);
        const BlockState* state = level->getBlockState(destroyPos);
        state->getBlock()->spawnResources(level, destroyPos, level->getBlockState(destroyPos), 0);
        level->setBlock(destroyPos, Blocks::air->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        level->setExtraBlock(destroyPos, Blocks::air->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        movedStates[--remaining] = state;
    }

    for (int i = toPush.size() - 1; i >= 0; --i)
    {
        BlockPos pushPos = toPush.at(i);
        const BlockState* state = level->getBlockState(pushPos);
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pushPos);
        CompoundTag* blockEntityTag = blockEntity != NULL ? blockEntity->getUpdateTagForPistonMovement() : NULL;
        level->setBlock(pushPos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->setExtraBlock(pushPos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->removeBlock(pushPos);
        pushPos = pushPos.relative(moveDirection);
        TypedBoxed<const Direction*> movingFacing(direction);
        level->setBlock(pushPos,
                        Blocks::piston_extension->defaultBlockState()->setBoxedValue(PistonMagicBlock::FACING,
                                                                                     &movingFacing),
                        Block::UPDATE_INVISIBLE, false);
        level->setBlockEntity(pushPos, PistonMagicBlock::newMovingBlockEntity(pushStates.at(i), direction, extending,
                                                                              false, blockEntityTag));
        movedStates[--remaining] = state;
    }

    if (extending)
    {
        PistonHeadBlock::PistonType* type = mSticky ? pistonTypeSticky : pistonTypeNormal;
        TypedBoxed<const Direction*> headFacing(direction);
        TypedBoxed<PistonHeadBlock::PistonType*> headType(type);
        const BlockState* headState = Blocks::piston_head->defaultBlockState()
                                          ->setBoxedValue(PistonHeadBlock::FACING, &headFacing)
                                          ->setBoxedValue(PistonHeadBlock::TYPE, &headType);
        TypedBoxed<const Direction*> movingFacing(direction);
        TypedBoxed<PistonHeadBlock::PistonType*> movingType(type);
        const BlockState* movingState = Blocks::piston_extension->defaultBlockState()
                                            ->setBoxedValue(PistonMagicBlock::FACING, &movingFacing)
                                            ->setBoxedValue(PistonMagicBlock::TYPE, &movingType);
        level->setBlock(headPos, movingState, Block::UPDATE_INVISIBLE, false);
        if (headInWater)
            level->setExtraBlock(headPos, Blocks::water->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        else
            level->setExtraBlock(headPos, Blocks::air->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
        level->setBlockEntity(headPos, PistonMagicBlock::newMovingBlockEntity(headState, direction, true, true, NULL));
    }

    for (int i = toDestroy.size() - 1; i >= 0; --i)
        level->updateNeighborsAt(toDestroy.at(i), movedStates[remaining++]->getBlock(), false);

    for (int i = toPush.size() - 1; i >= 0; --i)
        level->updateNeighborsAt(toPush.at(i), movedStates[remaining++]->getBlock(), false);

    if (extending)
    {
        level->updateNeighborsAt(headPos, Blocks::piston_head, false);
        level->neighborChanged(pos.below(), Blocks::piston_head, pos);
    }

    delete[] movedStates;
    return true;
}

bool PistonBaseBlock::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos,
                                   int eventId, int eventParam)
{
    const Direction* facing = state->getValue(FACING);
    if (!level->mIsClientSide)
    {
        bool extend = getNeighborSignal(level, pos, facing) != 0;
        if (extend && eventId == 1)
        {
            TypedBoxed<bool> extended(true);
            level->setBlock(pos, state->setBoxedValue(EXTENDED, &extended), Block::UPDATE_CLIENTS, false);
            return false;
        }
        if (!extend && eventId == 0)
            return false;
    }

    if (eventId == 0)
    {
        PIXBeginNamedEvent(0.0F, "Create push");
        if (!moveBlocks(level, pos, facing, true))
            return false;
        TypedBoxed<bool> extended(true);
        level->setBlock(pos, state->setBoxedValue(EXTENDED, &extended), Block::UPDATE_ALL, false);
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::PISTON_EXTEND, SoundSource::eSoundSource_Blocks,
                         0.5f, level->mRandom->nextFloat() * 0.25f + 0.6f, 16.0f);
        PIXEndNamedEvent();
    }
    else if (eventId == 1)
    {
        PIXBeginNamedEvent(0.0F, "Contract phase A");
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos.relative(facing));
        if (blockEntity != NULL && boost::dynamic_pointer_cast<PistonMovingBlockEntity>(blockEntity) != NULL)
            boost::dynamic_pointer_cast<PistonMovingBlockEntity>(blockEntity)->finalTick();

        TypedBoxed<const Direction*> movingFacing(facing);
        const BlockState* movingState = Blocks::piston_extension->defaultBlockState()->setBoxedValue(
            PistonMagicBlock::FACING, &movingFacing);
        TypedBoxed<PistonHeadBlock::PistonType*> type(mSticky ? pistonTypeSticky : pistonTypeNormal);
        level->setBlock(pos, movingState->setBoxedValue(PistonMagicBlock::TYPE, &type), Block::UPDATE_ALL, false);
        level->setBlockEntity(pos, PistonMagicBlock::newMovingBlockEntity(getBlockState(eventParam), facing, false,
                                                                          true, NULL));
        PIXEndNamedEvent();

        if (mSticky)
        {
            PIXBeginNamedEvent(0.0F, "Contract sticky phase A");
            BlockPos pullPos = pos.offset(facing->getStepX() * 2, facing->getStepY() * 2, facing->getStepZ() * 2);
            const BlockState* pullState = level->getBlockState(pullPos);
            Block* pullBlock = pullState->getBlock();
            bool pulledMovingBlock = false;
            PIXEndNamedEvent();
            if (pullBlock == Blocks::piston_extension)
            {
                PIXBeginNamedEvent(0.0F, "Contract sticky phase B");
                boost::shared_ptr<BlockEntity> pullEntity = level->getBlockEntity(pullPos);
                if (pullEntity != NULL && boost::dynamic_pointer_cast<PistonMovingBlockEntity>(pullEntity) != NULL)
                {
                    boost::shared_ptr<PistonMovingBlockEntity> movingEntity =
                        boost::dynamic_pointer_cast<PistonMovingBlockEntity>(pullEntity);
                    if (movingEntity->getDirection() == facing && movingEntity->isExtending())
                    {
                        movingEntity->finalTick();
                        pulledMovingBlock = true;
                    }
                }
                PIXEndNamedEvent();
            }

            PIXBeginNamedEvent(0.0F, "Contract sticky phase C");
            if (!pulledMovingBlock && pullState->getMaterial() != Material::AIR
                && isPushable(pullState, level, pullPos, facing->getOpposite(), false, facing)
                && (pullState->getPistonPushReaction() == PISTON_PUSH_NORMAL || pullBlock == Blocks::piston
                    || pullBlock == Blocks::sticky_piston))
            {
                moveBlocks(level, pos, facing, false);
            }
            PIXEndNamedEvent();
        }
        else
        {
            level->removeBlock(pos.relative(facing));
        }

        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::PISTON_CONTRACT,
                         SoundSource::eSoundSource_Blocks, 0.5f, level->mRandom->nextFloat() * 0.15f + 0.6f,
                         16.0f);
    }
    return true;
}

bool PistonBaseBlock::isCubeShaped(const BlockState*) { return false; }

const Direction* PistonBaseBlock::getFacing(int data)
{
    return (data & 7) < 6 ? Direction::from3DDataValue(data & 7) : NULL;
}

const BlockState* PistonBaseBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(getFacing(data));
    TypedBoxed<bool> extended((data & 8) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(FACING, &facing);
    return state->setBoxedValue(EXTENDED, &extended);
}

const BlockState* PistonBaseBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* PistonBaseBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

int PistonBaseBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue() | (state->getValue(EXTENDED) ? 8 : 0);
}

BlockStateDefinition* PistonBaseBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, EXTENDED };
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape PistonBaseBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                    const BlockPos& pos, const Direction* direction)
{
    const BlockState* virtualState = fillVirtualBlockStateProperties(state, levelSource, pos);
    if (virtualState->getValue(FACING)->getOpposite() != direction && virtualState->getValue(EXTENDED))
    {
        return BLOCK_FACE_UNDEFINED;
    }
    return BLOCK_FACE_SOLID;
}

TextureAtlasSprite* PistonBaseBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (direction != facing)
    {
        if (direction == facing->getOpposite())
            return mBottomIcon;
        return mIcon;
    }

    const AABB* shape = getShape(state, NULL, BlockPos());
    if (!state->getValue(EXTENDED) && shape->mMinX <= 0.0 && shape->mMinY <= 0.0 && shape->mMinZ <= 0.0 &&
        shape->mMaxX >= 1.0 && shape->mMaxY >= 1.0 && shape->mMaxZ >= 1.0)
        return mTopIcon;
    return mInnerTopIcon;
}

TextureAtlasSprite* PistonBaseBlock::getTexture(const std::wstring& name)
{
    if (name == L"piston_side")
        return static_cast<PistonBaseBlock*>(Blocks::piston)->mIcon;
    if (name == L"piston_top")
        return static_cast<PistonBaseBlock*>(Blocks::piston)->mTopIcon;
    if (name == L"piston_top_sticky")
        return static_cast<PistonBaseBlock*>(Blocks::sticky_piston)->mTopIcon;
    if (name == L"piston_inner_top")
        return static_cast<PistonBaseBlock*>(Blocks::piston)->mInnerTopIcon;
    return NULL;
}

void PistonBaseBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"piston_side");
    mTopIcon = iconRegister->registerIcon(mSticky ? L"piston_top_sticky" : L"piston_top");
    mInnerTopIcon = iconRegister->registerIcon(L"piston_inner_top");
    mBottomIcon = iconRegister->registerIcon(L"piston_bottom");
}

EBlockRenderShape PistonBaseBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_PISTON_BASE; }

bool PistonBaseBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType) { return true; }
