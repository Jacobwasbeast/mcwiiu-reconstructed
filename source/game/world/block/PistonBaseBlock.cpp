#include "game/world/block/PistonBaseBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
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
AbstractProperty<bool>* PistonBaseBlock::EXTENDED = NULL;

PistonBaseBlock::PistonBaseBlock(bool sticky)
    : DirectionalBlock(Material::PISTON), mSticky(sticky), mSideIcon(NULL), mTopIcon(NULL), mBottomIcon(NULL)
{
    DerivedInit();
    const BlockState* state = defaultBlockState();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> extended(false);
    if (state != NULL && FACING != NULL && EXTENDED != NULL)
    {
        state = state->setBoxedValue(FACING, &facing);
        state = state->setBoxedValue(EXTENDED, &extended);
        registerDefaultState(state);
    }
    setSoundType(NULL);
    setDestroyTime(0.5f);
}

PistonBaseBlock::~PistonBaseBlock()
{
}

void PistonBaseBlock::blockStaticCtor()
{
    if (FACING == NULL)
        FACING = DirectionProperty::create(L"facing");
    if (EXTENDED == NULL)
        EXTENDED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"extended"));
}

bool PistonBaseBlock::isViewBlocking(const BlockState* state)
{
    return !state->getValue(EXTENDED);
}

int PistonBaseBlock::getSpawnResourcesAuxValue(const BlockState*) { return 5; }

const AABB* PistonBaseBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getValue(EXTENDED))
        return NULL;
    return NULL;
}

bool PistonBaseBlock::isTopSolidBlocking(const BlockState* state)
{
    return !state->getValue(EXTENDED) && state->getValue(FACING) == Direction::DOWN;
}

void PistonBaseBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                        const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                        boost::shared_ptr<Entity> entity, bool isActualState)
{
    Block::addCollisionAABBs(state, level, pos, entityBox, collidingBoxes, entity, isActualState);
}

bool PistonBaseBlock::isSolidRender(const BlockState*) const { return false; }

int PistonBaseBlock::getNeighborSignal(Level* level, const BlockPos& pos, const Direction* direction)
{
    int signal = 0;
    std::vector<const Direction*> faces = Direction::VALUES;
    std::vector<const Direction*>::const_iterator it;
    for (it = faces.begin(); it != faces.end(); ++it)
    {
        if (*it != direction && level->hasSignal(pos.relative(*it), *it))
            signal = 15;
    }
    return signal;
}

bool PistonBaseBlock::IsPushRestrictedByLevelHeight(Level* level, const BlockPos& pos, const Direction* direction)
{
    return direction == Direction::UP && pos.y >= level->getMaxBuildHeight() - 1;
}

int PistonBaseBlock::isPushable(const BlockState* state, Level*, const BlockPos&, const Direction*, bool, const Direction*)
{
    if (state == NULL || state->getBlock() == NULL)
        return 0;
    if (state->getBlock()->getPistonPushReaction(state) == PISTON_PUSH_DESTROY)
        return 1;
    return 1;
}

void PistonBaseBlock::checkIfExtend(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    bool powered = getNeighborSignal(level, pos, facing) > 0;
    if (powered != state->getValue(EXTENDED))
        level->setBlock(pos, state->setBoxedValue(EXTENDED, new TypedBoxed<bool>(powered)), 2, false);
}

void PistonBaseBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    checkIfExtend(level, pos, state);
}

void PistonBaseBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                      Block*, const BlockPos&)
{
    checkIfExtend(level, pos, state);
}

void PistonBaseBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
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
    if (level == NULL || direction == NULL)
        return 0;
    if (!extending)
        level->neighborChanged(pos.relative(direction), this, pos);

    PistonStructureResolver resolver(level, pos, direction, extending);
    if (!resolver.resolve())
        return 0;

    std::vector<BlockPos>& toPush = resolver.getToPush();
    std::vector<BlockPos>& toDestroy = resolver.getToDestroy();
    const Direction* moveDirection = extending ? direction : direction->getOpposite();

    for (std::vector<BlockPos>::reverse_iterator it = toDestroy.rbegin(); it != toDestroy.rend(); ++it)
    {
        const BlockState* state = level->getBlockState(*it);
        Block* block = state == NULL ? NULL : state->getBlock();
        if (block != NULL)
            block->destroy(level, *it, state);
        level->setBlock(*it, Blocks::AIR->defaultBlockState(), 4, false);
        level->removeBlockEntity(*it);
    }

    for (std::vector<BlockPos>::reverse_iterator it = toPush.rbegin(); it != toPush.rend(); ++it)
    {
        const BlockPos source = *it;
        const BlockState* state = level->getBlockState(source);
        if (state == NULL)
            continue;
        CompoundTag* blockEntityTag = NULL;
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(source);
        if (blockEntity)
            blockEntityTag = blockEntity->getUpdateTagForPistonMovement();
        const BlockPos destination = source.relative(moveDirection);
        level->setBlock(source, Blocks::AIR->defaultBlockState(), 2, false);
        level->removeBlockEntity(source);
        boost::shared_ptr<BlockEntity> moving = PistonMagicBlock::newMovingBlockEntity(
            state, moveDirection, extending, false, blockEntityTag);
        level->setBlock(destination, Blocks::piston_extension->defaultBlockState(), 4, false);
        if (moving)
            level->setBlockEntity(destination, moving);
        level->neighborChanged(source, this, pos);
    }

    const BlockPos headPos = pos.relative(direction);
    const BlockState* headState = Blocks::piston_head->defaultBlockState();
    TypedBoxed<const Direction*> boxed(direction);
    if (PistonBaseBlock::FACING != NULL)
        headState = headState->setBoxedValue(PistonBaseBlock::FACING, &boxed);
    level->setBlock(headPos, headState, 4, false);
    if (extending)
    {
        level->updateNeighborsAt(headPos, this, false);
        level->neighborChanged(headPos.below(), this, pos);
    }
    return 1;
}

bool PistonBaseBlock::triggerEvent(const BlockState* state, Level* level, const BlockPos& pos,
                                   int eventId, int)
{
    if (eventId != 0 && eventId != 1)
        return false;
    return moveBlocks(level, pos, state->getValue(FACING), eventId == 0) != 0;
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

EBlockFaceShape PistonBaseBlock::getBlockFaceShape(LevelSource*, const BlockState* state,
                                                    const BlockPos&, const Direction* direction)
{
    return state->getValue(FACING)->getOpposite() == direction ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* PistonBaseBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == state->getValue(FACING))
        return mTopIcon;
    if (direction == state->getValue(FACING)->getOpposite())
        return mBottomIcon;
    return mSideIcon;
}

TextureAtlasSprite* PistonBaseBlock::getTexture(const std::wstring& name)
{
    PistonBaseBlock* piston = dynamic_cast<PistonBaseBlock*>(Blocks::piston);
    PistonBaseBlock* sticky = dynamic_cast<PistonBaseBlock*>(Blocks::sticky_piston);
    if (name == L"piston_top")
        return piston != NULL ? piston->mTopIcon : NULL;
    if (name == L"piston_bottom")
        return piston != NULL ? piston->mBottomIcon : NULL;
    if (name == L"piston_side")
        return piston != NULL ? piston->mSideIcon : NULL;
    if (name == L"piston_side_sticky")
        return sticky != NULL ? sticky->mSideIcon : NULL;
    return NULL;
}

void PistonBaseBlock::registerIcons(IconRegister* iconRegister)
{
    mSideIcon = iconRegister->registerIcon(mSticky ? L"piston_side_sticky" : L"piston_side");
    mTopIcon = iconRegister->registerIcon(L"piston_top");
    mBottomIcon = iconRegister->registerIcon(L"piston_bottom");
}

EBlockRenderShape PistonBaseBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_ENTITY; }

bool PistonBaseBlock::canProvideSupport(const BlockState*, const Direction*, EBlockSupportType) { return true; }
