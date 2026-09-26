#include "game/world/block/FarmBlock.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"

AbstractProperty<int>* FarmBlock::MOISTURE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"moisture", 0, 7));
static const AABB* gFarmShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.9375, 1.0);

FarmBlock::FarmBlock()
    : Block(Material::DIRT)
{
    DerivedInit();
    TypedBoxed<int> moisture(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(MOISTURE, &moisture));
    setTicking(true);
    setLightBlock(0xff);
}

FarmBlock::~FarmBlock() {}

const AABB* FarmBlock::getShape(const BlockState*, LevelSource*, const BlockPos&) { return gFarmShape; }
bool FarmBlock::isSolidRender(const BlockState*) const { return false; }
bool FarmBlock::isCubeShaped(const BlockState*) { return false; }

void FarmBlock::turnToDirt(Level* level, const BlockPos& pos)
{
    level->setBlock(pos, Blocks::dirt->defaultBlockState(), Block::UPDATE_ALL, false);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(Entity), getShape(defaultBlockState(), level, pos));
    if (entities == NULL) return;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        if (it->get() != NULL && it->get()->mBbWidth * it->get()->mBbWidth * it->get()->mBbHeight > 0.512f)
            it->get()->causeFallDamage(0.0f, 0.0f);
    }
}

void FarmBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    int moisture = convertBlockStateToLegacyData(state);
    if (isUnderCrops(level, pos) || level->isRainingAt(pos.above()))
    {
        if (moisture < 7) level->setBlock(pos, getBlockState(7), Block::UPDATE_CLIENTS, false);
    }
    else if (moisture > 0)
        level->setBlock(pos, getBlockState(moisture - 1), Block::UPDATE_CLIENTS, false);
    else if (!isNearWater(level, pos))
        turnToDirt(level, pos);
}

void FarmBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    if (entity && !entity->mOnGround && entity->mRandom->nextFloat() < fallDistance - 0.5f)
    {
        unsigned long long type = entity->GetType();
        if ((type & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY
            && ((type & EntityType::PLAYER) == EntityType::PLAYER || level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
            && entity->mBbWidth * entity->mBbWidth * entity->mBbHeight > 0.512f)
            turnToDirt(level, pos);
    }
    Block::fallOn(level, pos, entity, fallDistance);
}

bool FarmBlock::isUnderCrops(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos.above());
    return state != NULL && state->getBlock()->getRenderShape(state) == BLOCK_RENDER_MODEL;
}

bool FarmBlock::isNearWater(Level* level, const BlockPos& pos)
{
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(pos.offset(-4, 0, -4),
                                                                        pos.offset(4, 1, 4));
    while (positions->hasNext())
    {
        MutableBlockPos* current = positions->next();
        if (level->getBlockState(*current)->getMaterial() == Material::WATER)
        {
            delete positions;
            return true;
        }
    }
    delete positions;
    return false;
}

void FarmBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    Block::neighborChanged(state, level, pos, block, fromPos);
    if (level->getBlockState(pos.above())->getMaterial()->blocksMotion()) turnToDirt(level, pos);
}

void FarmBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    if (level->getBlockState(pos.above())->getMaterial()->blocksMotion()) turnToDirt(level, pos);
}

bool FarmBlock::shouldRenderFace(const BlockState* state, LevelSource* source, const BlockPos& pos, const Direction* direction)
{
    int data = direction->get3DDataValue();
    if (data == Direction::eDirection_Down) return Block::shouldRenderFace(state, source, pos, direction);
    if (data == Direction::eDirection_Up) return true;
    if (data < 6)
    {
        const BlockState* neighbour = source->getBlockState(pos.relative(direction));
        Block* block = neighbour->getBlock();
        return !(neighbour->isSolidRender() || block == Blocks::farmland || block == Blocks::grass_path);
    }
    return Block::shouldRenderFace(state, source, pos, direction);
}

Item* FarmBlock::getResource(const BlockState*, Random* random, int lootBonus)
{
    return Blocks::dirt->getResource(Blocks::dirt->defaultBlockState(), random, lootBonus);
}

const BlockState* FarmBlock::getBlockState(int data)
{
    TypedBoxed<int> moisture(data & 7);
    return defaultBlockState()->setBoxedValue(MOISTURE, &moisture);
}

int FarmBlock::convertBlockStateToLegacyData(const BlockState* state) { return state->getValue(MOISTURE); }
bool FarmBlock::canContainLiquid(const BlockState*) const { return false; }

not_null_ptr<ItemInstance> FarmBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::dirt), 1, getSpawnResourcesAuxValue(state)));
}

BlockStateDefinition* FarmBlock::createBlockStateDefinition()
{
    const Property* properties[] = { MOISTURE };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape FarmBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* FarmBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction != Direction::UP) return Blocks::dirt->getTexture(direction);
    return state->getValue(MOISTURE) > 0 ? mWetIcon : mDryIcon;
}

void FarmBlock::registerIcons(IconRegister* iconRegister)
{
    mWetIcon = iconRegister->registerIcon(L"farmland_wet");
    mDryIcon = iconRegister->registerIcon(L"farmland_dry");
}
