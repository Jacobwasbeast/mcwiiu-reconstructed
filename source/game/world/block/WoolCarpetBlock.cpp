#include "game/world/block/Predicates.hpp"
#include "game/world/block/WoolCarpetBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<const DyeColor*>* gWoolCarpetColorProperty = NULL;
static const AABB* WOOL_CARPET_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.0625, 1.0);

void WoolCarpetBlock::blockStaticCtor()
{
    boost::unordered_set<const DyeColor*> colors;
    for (int data = 0; data < 16; ++data)
    {
        colors.insert(DyeColor::byBlockData(data));
    }

    gWoolCarpetColorProperty = EnumProperty<const DyeColor*>::create(
        L"color", typeid(DyeColor), colors, Predicates<const DyeColor*>::alwaysTrue);
}

WoolCarpetBlock::WoolCarpetBlock()
    : Block(Material::CLOTH_DECORATION)
{
    DerivedInit();
    TypedBoxed<const DyeColor*> color(DyeColor::WHITE);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gWoolCarpetColorProperty, &color));
    setTicking(true);
    setThickness(0.07f);
}

WoolCarpetBlock::~WoolCarpetBlock()
{
}

void WoolCarpetBlock::addCollisionAABB(const BlockPos& pos, const AABB* entityBox,
                                       std::vector<AABB*>* collidingBoxes, const AABB* blockBox)
{
    AABB* movedBox = WOOL_CARPET_SHAPE->move(pos);
    if (entityBox->intersects(movedBox->mMinX, movedBox->mMinY, movedBox->mMinZ,
                              movedBox->mMaxX, movedBox->mMaxY, movedBox->mMaxZ))
    {
        collidingBoxes->push_back(movedBox);
    }
}

const AABB* WoolCarpetBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return WOOL_CARPET_SHAPE;
}

const MaterialColor* WoolCarpetBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return MaterialColor::getDyeMaterial(state->getValue(gWoolCarpetColorProperty));
}

bool WoolCarpetBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool WoolCarpetBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool WoolCarpetBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && canSurvive(level, pos, NULL);
}

bool WoolCarpetBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!canSurvive(level, pos, state))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
        return false;
    }
    return true;
}

void WoolCarpetBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                      Block*, const BlockPos&)
{
    checkCanSurvive(level, pos, state);
}

bool WoolCarpetBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return !level->isEmptyBlock(pos.below());
}

bool WoolCarpetBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource,
                                       const BlockPos& pos, const Direction* direction)
{
    if (direction != Direction::DOWN)
    {
        Block* neighbour = levelSource->getBlock(pos.relative(direction));
        if (neighbour != this)
        {
            return Block::shouldRenderFace(state, levelSource, pos, direction);
        }
    }
    return true;
}

int WoolCarpetBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(gWoolCarpetColorProperty)->getBlockData();
}

const BlockState* WoolCarpetBlock::getBlockState(int data)
{
    TypedBoxed<const DyeColor*> color(DyeColor::byBlockData(data));
    return defaultBlockState()->setBoxedValue(gWoolCarpetColorProperty, &color);
}

int WoolCarpetBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gWoolCarpetColorProperty)->getBlockData();
}

BlockStateDefinition* WoolCarpetBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gWoolCarpetColorProperty };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape WoolCarpetBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* WoolCarpetBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int data = state->getValue(gWoolCarpetColorProperty)->getBlockData();
    return Blocks::wool->getTexture(direction, Blocks::wool->getBlockState(data));
}

void WoolCarpetBlock::registerIcons(IconRegister*)
{
}

bool WoolCarpetBlock::hasInHandRenderOffset()
{
    return true;
}

bool WoolCarpetBlock::waterSpreadCausesSpawn()
{
    return true;
}
