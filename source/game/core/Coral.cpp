#include "game/core/Coral.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

extern Property* Coral_VARIANT;

Coral::Coral()
    : Bush(Material::CORAL)
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
    return Block::getColor(state);
}

int Coral::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    return Block::getColor(levelSource, pos, state);
}

const MaterialColor* Coral::getMapColor(const BlockState* state, LevelSource* levelSource,
                                        const BlockPos& pos)
{
    return Block::getMapColor(state, levelSource, pos);
}

Item* Coral::getResource(const BlockState*, Random*, int)
{
    return NULL;
}

not_null_ptr<ItemInstance> Coral::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::coral, 1,
                                                       convertBlockStateToLegacyData(state)));
}

bool Coral::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

void Coral::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                            Block*, const BlockPos& fromPos)
{
    if (!level->mIsClientSide)
    {
        BlockPos below = fromPos.below();
        if (!level->isSolidBlockingBlock(below))
        {
            const BlockState* belowState = level->getBlockState(below);
            if (!belowState->getBlock()->mayPlace(level, fromPos))
            {
                level->setBlock(fromPos, Blocks::AIR->defaultBlockState(), 3, false);
            }
        }
        level->levelEvent(10, fromPos, 0);
    }
}

TextureAtlasSprite* Coral::getTexture(const Direction*, const BlockState* state)
{
    int variant = static_cast<TypedBoxed<int>*>(state->getBoxedValue(Coral_VARIANT))->getValue();
    return mIcons[variant];
}

void Coral::registerIcons(IconRegister* iconRegister)
{
    mIcons[0] = iconRegister->registerIcon(L"coral_red");
    mIcons[1] = iconRegister->registerIcon(L"coral_orange");
    mIcons[2] = iconRegister->registerIcon(L"coral_yellow");
    mIcons[3] = iconRegister->registerIcon(L"coral_blue");
    mIcons[4] = iconRegister->registerIcon(L"coral_purple");
}

int Coral::convertBlockStateToLegacyData(const BlockState* state)
{
    int variant = static_cast<TypedBoxed<int>*>(state->getBoxedValue(Coral_VARIANT))->getValue();
    return variant;
}

const BlockState* Coral::getBlockState(int data)
{
    TypedBoxed<int> variant(data);
    return defaultBlockState()->setBoxedValue(Coral_VARIANT, &variant);
}

int Coral::getDescriptionId(int auxValue)
{
    static const unsigned int ids[5] = { 0U, 0x1A417707U, 0x1B1CB5E5U, 0x27B09D40U, 0xA76B491FU };
    if (auxValue != 0 && auxValue < 5)
    {
        return static_cast<int>(ids[auxValue]);
    }
    return 0x25D640BD;
}

int Coral::getUseDescriptionId(int)
{
    return 0x226A4AE2;
}

const AABB* Coral::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

BlockStateDefinition* Coral::createBlockStateDefinition()
{
    const Property* properties[1] = { Coral_VARIANT };
    return new BlockStateDefinition(this, properties, 1);
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

    const BlockState* aboveState = level->getBlockState(pos.above(2));
    if (aboveState->getMaterial() != Material::WATER)
    {
        const BlockState* currentState = level->getBlockState(pos);
        if (currentState->getMaterial() == Material::CORAL)
        {
            int variant = static_cast<TypedBoxed<int>*>(currentState->getBoxedValue(Coral_VARIANT))->getValue();
            if (static_cast<unsigned int>(variant - 1) < 7U)
            {
                return false;
            }
        }

        bool validSupport = false;
        Block* block = state->getBlock();
        if (block->mayPlace(level, pos))
        {
            int data = block->convertBlockStateToLegacyData(state);
            if (block->isSolidRender(state) && !HalfSlabBlock::isBottomSlab(data))
            {
                validSupport = true;
            }
        }
        if (state->isSolidBlockingBlock() || validSupport)
        {
            return true;
        }
    }
    return false;
}

void Coral::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (level->getBlockState(pos)->getMaterial() == mMaterial)
    {
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
    }
}
