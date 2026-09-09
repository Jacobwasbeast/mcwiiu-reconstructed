#include "game/core/Coral.hpp"

#include <string>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/IntegerProperty.hpp"

AbstractProperty<int>* Coral_VARIANT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variant", 0, 4));

namespace
{
    std::wstring CORAL_ICON_NAMES[5] =
    {
        L"coral_red", L"coral_orange", L"coral_yellow", L"coral_blue", L"coral_purple"
    };

    const AABB* CORAL_SHAPE = NULL;
}

Coral::Coral()
    : Bush(Material::REPLACEABLE_PLANT)
{
    DerivedInit();

    for (int i = 0; i < 5; ++i)
    {
        mIcons[i] = NULL;
    }
}

Coral::~Coral()
{
}

int Coral::getColor(const BlockState* state)
{
    return Blocks::grass->getColor(state);
}

int Coral::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Blocks::grass->getColor(levelSource, pos, state);
}

const MaterialColor* Coral::getMapColor(const BlockState* state, LevelSource* levelSource,
                                        const BlockPos& pos)
{
    return Blocks::grass->getMapColor(state, levelSource, pos);
}

Item* Coral::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

not_null_ptr<ItemInstance> Coral::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                       const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1,
                                                       convertBlockStateToLegacyData(state)));
}

bool Coral::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

void Coral::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                            Block* block, const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    if (!level->isSolidBlockingBlock(pos.below()) && !level->getBlock(pos.below())->isSlabBlock())
    {
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
    }

    level->addToTickNextTick(pos, this, 10);
}

TextureAtlasSprite* Coral::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(Coral_VARIANT)];
}

void Coral::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 5; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(CORAL_ICON_NAMES[i]);
    }
}

int Coral::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(Coral_VARIANT);
}

const BlockState* Coral::getBlockState(int data)
{
    TypedBoxed<int> variant(data);
    return defaultBlockState()->setBoxedValue(Coral_VARIANT, &variant);
}

int Coral::getDescriptionId(int auxValue)
{
    static const int DESCRIPTION_IDS[5] =
    {
        0, 0x1A417707, 0x1B1CB5E5, 0x27B09D40, static_cast<int>(0xA76B491FU)
    };

    if (auxValue != 0 && static_cast<unsigned int>(auxValue) < 5U)
    {
        return DESCRIPTION_IDS[auxValue];
    }

    return 0x25D640BD;
}

int Coral::getUseDescriptionId(int auxValue)
{
    return 0x226A4AE2;
}

const AABB* Coral::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return CORAL_SHAPE;
}

BlockStateDefinition* Coral::createBlockStateDefinition()
{
    static const Property* PROPERTIES[1] = { Coral_VARIANT };

    return new BlockStateDefinition(this, PROPERTIES, 1);
}

bool Coral::isSilkTouchable()
{
    return true;
}

bool Coral::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->isUnderWater(pos.above(), 263, 0))
    {
        return false;
    }

    if (level->getBlock(pos.above(2))->mMaterial == Material::AIR)
    {
        return false;
    }

    if (level->getLiquidBlock(pos)->mMaterial == Material::WATER)
    {
        int liquidLevel = level->getLiquidBlockState(pos)->getValue(LiquidBlock::LEVEL);
        if (static_cast<unsigned int>(liquidLevel - 1) < 7U)
        {
            return false;
        }
    }

    bool onSlabTop = false;
    Block* block = state->getBlock();

    if (block->isSlabBlock()
        && !HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state)))
    {
        onSlabTop = true;
    }

    return state->canProvideSupport(Direction::UP, BLOCK_SUPPORT_CENTER) || onSlabTop;
}

void Coral::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::FLOWING_WATER->defaultBlockState(), 3, false);
    }
}
