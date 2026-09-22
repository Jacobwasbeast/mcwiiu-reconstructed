#include "game/world/block/ButtonBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* ButtonBlock::FACING = NULL;
AbstractProperty<bool>* ButtonBlock::POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
const AABB* ButtonBlock::AABB_DOWN_OFF = AABB::newPermanent(0.3125, 0.875, 0.375, 0.6875, 1.0, 0.625);
const AABB* ButtonBlock::AABB_UP_OFF = AABB::newPermanent(0.3125, 0.0, 0.375, 0.6875, 0.125, 0.625);
const AABB* ButtonBlock::AABB_NORTH_OFF = AABB::newPermanent(0.3125, 0.375, 0.875, 0.6875, 0.625, 1.0);
const AABB* ButtonBlock::AABB_SOUTH_OFF = AABB::newPermanent(0.3125, 0.375, 0.0, 0.6875, 0.625, 0.125);
const AABB* ButtonBlock::AABB_WEST_OFF = AABB::newPermanent(0.875, 0.375, 0.3125, 1.0, 0.625, 0.6875);
const AABB* ButtonBlock::AABB_EAST_OFF = AABB::newPermanent(0.0, 0.375, 0.3125, 0.125, 0.625, 0.6875);
const AABB* ButtonBlock::AABB_DOWN_ON = AABB::newPermanent(0.3125, 0.9375, 0.375, 0.6875, 1.0, 0.625);
const AABB* ButtonBlock::AABB_UP_ON = AABB::newPermanent(0.3125, 0.0, 0.375, 0.6875, 0.0625, 0.625);
const AABB* ButtonBlock::AABB_NORTH_ON = AABB::newPermanent(0.3125, 0.375, 0.9375, 0.6875, 0.625, 1.0);
const AABB* ButtonBlock::AABB_SOUTH_ON = AABB::newPermanent(0.3125, 0.375, 0.0, 0.6875, 0.625, 0.0625);
const AABB* ButtonBlock::AABB_WEST_ON = AABB::newPermanent(0.9375, 0.375, 0.3125, 1.0, 0.625, 0.6875);
const AABB* ButtonBlock::AABB_EAST_ON = AABB::newPermanent(0.0, 0.375, 0.3125, 0.0625, 0.625, 0.6875);

ButtonBlock::ButtonBlock(bool wooden, Material* material)
    : Block(material), mWooden(wooden)
{
    setTicking(true);
}

ButtonBlock::~ButtonBlock()
{
}

void ButtonBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

void ButtonBlock::DerivedInit()
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(FACING, &facing);
    TypedBoxed<bool> powered(false);
    registerDefaultState(state->setBoxedValue(POWERED, &powered));
}

const AABB* ButtonBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

int ButtonBlock::getTickDelay(Level*)
{
    return mWooden ? 30 : 20;
}

bool ButtonBlock::isSolidRender(const BlockState*) const { return false; }
bool ButtonBlock::isCubeShaped(const BlockState*) { return false; }

bool ButtonBlock::canAttachTo(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos supportPos = pos.relative(direction->getOpposite());
    const BlockState* supportState = level->getBlockState(supportPos);
    EBlockFaceShape faceShape = supportState->getBlockFaceShape(level, supportPos, direction);
    Block* support = supportState->getBlock();
    if (direction == Direction::UP)
    {
        if (support != Blocks::AIR && (Block::isExceptionBlockForAttaching(support) || faceShape != BLOCK_FACE_SOLID) &&
            !Block::isExceptionForPistonAttaching(support) && !support->mHasTintedFace) return false;
        return true;
    }
    if ((Block::isExceptionBlockForAttaching(support) || faceShape != BLOCK_FACE_SOLID) && !Block::isExceptionForPistonAttaching(support)) return false;
    return true;
}

bool ButtonBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return canAttachTo(level, pos, direction);
}

bool ButtonBlock::mayPlace(Level* level, const BlockPos& pos)
{
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
        if (canAttachTo(level, pos, *it)) return true;
    return false;
}

const BlockState* ButtonBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    TypedBoxed<const Direction*> facing(direction);
    TypedBoxed<bool> powered(false);
    const BlockState* state = defaultBlockState();
    if (!canAttachTo(level, pos, direction)) facing = TypedBoxed<const Direction*>(Direction::DOWN);
    state = state->setBoxedValue(FACING, &facing);
    return state->setBoxedValue(POWERED, &powered);
}

bool ButtonBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (mayPlace(level, pos)) return true;
    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
    return false;
}

void ButtonBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (checkCanSurvive(level, pos, state) && !canAttachTo(level, pos, state->getValue(FACING)))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

int ButtonBlock::getSpawnResourcesAuxValue(const BlockState*) { return 4; }

const AABB* ButtonBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* direction = state->getValue(FACING);
    bool powered = state->getValue(POWERED);
    if (direction == Direction::DOWN) return powered ? AABB_DOWN_ON : AABB_DOWN_OFF;
    if (direction == Direction::UP) return powered ? AABB_UP_ON : AABB_UP_OFF;
    if (direction == Direction::SOUTH) return powered ? AABB_SOUTH_ON : AABB_SOUTH_OFF;
    if (direction == Direction::WEST) return powered ? AABB_WEST_ON : AABB_WEST_OFF;
    if (direction == Direction::EAST) return powered ? AABB_EAST_ON : AABB_EAST_OFF;
    return powered ? AABB_NORTH_ON : AABB_NORTH_OFF;
}
bool ButtonBlock::TestUse() { return true; }

void ButtonBlock::updateNeighbours(Level* level, const BlockPos& pos, const Direction* direction)
{
    level->updateNeighborsAt(pos, this, false);
    level->updateNeighborsAt(pos.relative(direction->getOpposite()), this, false);
}

bool ButtonBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        playOnSound(player, level, pos);
        return false;
    }
    if (state->getValue(POWERED)) return true;
    TypedBoxed<bool> powered(true);
    level->setBlock(pos, state->setBoxedValue(POWERED, &powered), 3, false);
    level->setBlocksDirty(pos, pos);
    playOnSound(player, level, pos);
    updateNeighbours(level, pos, state->getValue(FACING));
    level->addToTickNextTick(pos, this, getTickDelay(level));
    return true;
}

void ButtonBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(POWERED)) updateNeighbours(level, pos, state->getValue(FACING));
}

int ButtonBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return state->getValue(POWERED) ? 15 : 0;
}

int ButtonBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    return state->getValue(POWERED) && direction == state->getValue(FACING) ? 15 : 0;
}

bool ButtonBlock::isSignalSource(const BlockState*) { return true; }
void ButtonBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*) {}

void ButtonBlock::checkPressed(const BlockState* state, Level* level, const BlockPos& pos)
{
    const AABB* shape = state->getShape(level, pos);
    std::vector<boost::shared_ptr<Entity> >* arrows =
        level->getEntitiesOfClass(typeid(Arrow), shape->move(pos));
    std::vector<boost::shared_ptr<Entity> >* tridents =
        level->getEntitiesOfClass(typeid(ThrownTrident), shape->move(pos));
    bool pressed = !arrows->empty() || !tridents->empty();
    bool powered = state->getValue(POWERED);
    delete arrows;
    delete tridents;

    if (pressed)
    {
        if (!powered)
        {
            TypedBoxed<bool> poweredValue(true);
            level->setBlockAndUpdate(pos, state->setBoxedValue(POWERED, &poweredValue));
            updateNeighbours(level, pos, state->getValue(FACING));
            level->setBlocksDirty(pos, pos);
            playOnSound(boost::shared_ptr<Player>(), level, pos);
        }
        level->addToTickNextTick(pos, this, getTickDelay(level));
    }
    else if (powered)
    {
        TypedBoxed<bool> poweredValue(false);
        level->setBlockAndUpdate(pos, state->setBoxedValue(POWERED, &poweredValue));
        updateNeighbours(level, pos, state->getValue(FACING));
        level->setBlocksDirty(pos, pos);
        playOffSound(level, pos);
    }
}

void ButtonBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && state->getValue(POWERED))
    {
        if (mWooden) checkPressed(state, level, pos);
        else
        {
            TypedBoxed<bool> powered(false);
            level->setBlockAndUpdate(pos, state->setBoxedValue(POWERED, &powered));
            updateNeighbours(level, pos, state->getValue(FACING));
            playOffSound(level, pos);
            level->setBlocksDirty(pos, pos);
        }
    }
}

void ButtonBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide && mWooden && !state->getValue(POWERED)) checkPressed(state, level, pos);
}

const BlockState* ButtonBlock::getBlockState(int data)
{
    static const Direction* const legacyFacing[] = {
        Direction::DOWN, Direction::EAST, Direction::WEST,
        Direction::SOUTH, Direction::NORTH, Direction::UP
    };
    unsigned int facingIndex = static_cast<unsigned int>(data) & 7u;
    const Direction* direction = facingIndex < 6u ? legacyFacing[facingIndex] : Direction::UP;
    TypedBoxed<const Direction*> facing(direction);
    TypedBoxed<bool> powered((data & 8) != 0);
    return defaultBlockState()->setBoxedValue(FACING, &facing)->setBoxedValue(POWERED, &powered);
}

int ButtonBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    const Direction* direction = state->getValue(FACING);
    int data;
    if (direction == Direction::DOWN) data = 0;
    else if (direction == Direction::UP) data = 5;
    else if (direction == Direction::NORTH) data = 4;
    else if (direction == Direction::SOUTH) data = 3;
    else if (direction == Direction::WEST) data = 2;
    else data = 1;
    return data | (state->getValue(POWERED) ? 8 : 0);
}

unsigned int ButtonBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside) return 0xea4e897e;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* ButtonBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* ButtonBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* ButtonBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, POWERED };
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape ButtonBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool ButtonBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState* state)
{
    return state->getValue(POWERED);
}
