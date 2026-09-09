#include "game/world/block/SkullBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockMaterialPredicate.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPatternBuilder.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

bool isWitherSkull(BlockInWorld* block);

bool isWitherSkull(BlockInWorld* block)
{
    const BlockState* state = block->getState();
    if (state == NULL || state->getBlock() != Blocks::skull)
        return false;

    boost::shared_ptr<BlockEntity> entity = block->getEntity();
    if (!entity || entity->GetType() != 0x0120020700000000ULL)
        return false;

    boost::shared_ptr<SkullBlockEntity> skull = boost::dynamic_pointer_cast<SkullBlockEntity>(entity);
    return skull && skull->getSkullType() == 1;
}

DirectionProperty* SkullBlock::FACING = NULL;
AbstractProperty<bool>* SkullBlock::WATERLOGGED = NULL;

static const AABB* SKULL_AABB = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 0.5, 0.75);
static const AABB* SKULL_NORTH_AABB = AABB::newPermanent(0.25, 0.25, 0.5, 0.75, 0.75, 1.0);
static const AABB* SKULL_SOUTH_AABB = AABB::newPermanent(0.25, 0.25, 0.0, 0.75, 0.75, 0.5);
static const AABB* SKULL_WEST_AABB = AABB::newPermanent(0.5, 0.25, 0.25, 1.0, 0.75, 0.75);
static const AABB* SKULL_EAST_AABB = AABB::newPermanent(0.0, 0.25, 0.25, 0.5, 0.75, 0.75);

SkullBlock::SkullBlock()
    : BaseEntityBlock(Material::DECORATION), mWitherBase(NULL), mWitherFull(NULL)
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(FACING, &facing);
    TypedBoxed<bool> waterlogged(false);
    registerDefaultState(state->setBoxedValue(WATERLOGGED, &waterlogged));
}

SkullBlock::~SkullBlock()
{
}

boost::shared_ptr<BlockEntity> SkullBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<BlockEntity>(new SkullBlockEntity());
}

void SkullBlock::blockStaticCtor()
{
    FACING = DirectionProperty::create(L"facing");
    WATERLOGGED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"waterlogged"));
}

std::wstring SkullBlock::getName()
{
    return std::wstring(L10N::GetString(0xfe1de21a));
}

bool SkullBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool SkullBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool SkullBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}

const AABB* SkullBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case 2: return SKULL_NORTH_AABB;
    case 3: return SKULL_SOUTH_AABB;
    case 4: return SKULL_WEST_AABB;
    case 5: return SKULL_EAST_AABB;
    default: return SKULL_AABB;
    }
}

const BlockState* SkullBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float,
                                                    float, int, boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = Direction::getDirectionFromLivingEntity(pos, placer);
    TypedBoxed<const Direction*> boxedFacing(facing);
    TypedBoxed<bool> waterlogged(false);
    const BlockState* state = defaultBlockState()->setBoxedValue(FACING, &boxedFacing);
    return state->setBoxedValue(WATERLOGGED, &waterlogged);
}

not_null_ptr<ItemInstance> SkullBlock::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                            const BlockState*)
{
    int skullType = 0;
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (entity && entity->GetType() == 0x01200207ULL)
    {
        boost::shared_ptr<SkullBlockEntity> skull = boost::dynamic_pointer_cast<SkullBlockEntity>(entity);
        if (skull)
            skullType = skull->getSkullType();
    }
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, skullType));
}

void SkullBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state,
                                float chance, int lootBonus)
{
}

bool SkullBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player)
{
    if (player && player->mAbilities.mInstabuild)
    {
        TypedBoxed<bool> waterlogged(true);
        level->setBlock(pos, state->setBoxedValue(WATERLOGGED, &waterlogged), 4, false);
    }
    return Block::playerWillDestroy(level, pos, state, player);
}

void SkullBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide && !state->getValue(WATERLOGGED))
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (entity && entity->GetType() == 0x01200207ULL)
            Block::popResource(level, pos, getCloneItemInstance(level, pos, state));
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

Item* SkullBlock::getResource(const BlockState*, Random*, int)
{
    return Items::skull;
}

BlockPattern* SkullBlock::getOrCreateWitherBase()
{
    if (mWitherBase == NULL)
    {
        std::wstring aisle[] = { L"   ", L"###", L"~#~" };
        mWitherBase = BlockPatternBuilder::start()->aisle(aisle)
            ->where('#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::soul_sand)))
            ->where('~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::AIR)))
            ->build(NULL, NULL);
    }
    return mWitherBase;
}

BlockPattern* SkullBlock::getOrCreateWitherFull()
{
    if (mWitherFull == NULL)
    {
        std::wstring aisle[] = { L"^^^", L"###", L"~#~" };
        mWitherFull = BlockPatternBuilder::start()->aisle(aisle)
            ->where('#', BlockInWorld::hasState(BlockStatePredicate::forBlock(Blocks::soul_sand)))
            ->where('^', new Predicates<BlockInWorld*>::FunctionPredicate(isWitherSkull))
            ->where('~', BlockInWorld::hasState(BlockMaterialPredicate::forMaterial(Material::AIR)))
            ->build(NULL, NULL);
    }
    return mWitherFull;
}

bool SkullBlock::canSpawnMob(Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> item)
{
    if (item->getAuxValue() != 1 || pos.y <= 1 || level->getDifficulty() == Difficulty::PEACEFUL
        || level->mIsClientSide)
        return false;
    BlockPattern::BlockPatternMatch* match = getOrCreateWitherBase()->find(level, pos);
    bool found = match != NULL;
    delete match;
    return found;
}

void SkullBlock::checkMobSpawn(Level* level, const BlockPos& pos,
                               boost::shared_ptr<SkullBlockEntity> skull,
                               boost::shared_ptr<Player>)
{
    if (!skull || skull->getSkullType() != 1 || pos.y <= 1 || level->mIsClientSide)
        return;
    BlockPattern::BlockPatternMatch* match = getOrCreateWitherFull()->find(level, pos);
    if (match == NULL)
        return;
    if (level->getDifficulty() == Difficulty::PEACEFUL)
    {
        delete match;
        return;
    }
    bool canCreate = level->canCreateMore(0x000000005d540047ULL, 0);
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            BlockInWorld* block = match->getBlock(x, y, 0);
            BlockPos blockPos = block->getPos();
            if (!canCreate)
                level->blockUpdated(blockPos, block->getState()->getBlock(), false);
            level->setBlock(blockPos, Blocks::AIR->defaultBlockState(), 2, false);
        }
    }
    if (canCreate)
    {
        BlockPos spawn = match->getBlock(1, 2, 0)->getPos();
        boost::shared_ptr<WitherBoss> wither(new WitherBoss(level));
        wither->moveTo(spawn.x + 0.5, spawn.y + 0.55, spawn.z + 0.5, 0.0f, 0.0f);
        wither->makeInvulnerable();
        level->addEntity(wither);
    }
    delete match;
}

const BlockState* SkullBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from3DDataValue(data & 7));
    TypedBoxed<bool> waterlogged((data & 8) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(FACING, &facing);
    return state->setBoxedValue(WATERLOGGED, &waterlogged);
}

int SkullBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue() | (state->getValue(WATERLOGGED) ? 8 : 0);
}

bool SkullBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool SkullBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

const BlockState* SkullBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* SkullBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* SkullBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, WATERLOGGED };
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape SkullBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return static_cast<EBlockFaceShape>(8);
}

void SkullBlock::registerIcons(IconRegister*)
{
}

TextureAtlasSprite* SkullBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return Blocks::soul_sand->getTexture(direction, state);
}

std::wstring SkullBlock::getTileItemIconName()
{
    return L"";
}

EBlockRenderShape SkullBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(-1);
}
