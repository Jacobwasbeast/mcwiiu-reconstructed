#include "game/world/block/Predicates.hpp"
#include "game/world/block/TallGrassBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

AbstractProperty<TallGrassBlock::Variant*>* TallGrassBlock::VARIANT = NULL;
TallGrassBlock::Variant* TallGrassBlock::VARIANTS[3] = { NULL, NULL, NULL };
TallGrassBlock::Variant* TallGrassBlock::Variant::DEAD_BUSH = NULL;
TallGrassBlock::Variant* TallGrassBlock::Variant::GRASS = NULL;
TallGrassBlock::Variant* TallGrassBlock::Variant::FERN = NULL;
TallGrassBlock::Variant* gTallGrass = NULL;
TallGrassBlock::Variant* gFern = NULL;
Property* TallGrassBlock_VARIANT = NULL;

TallGrassBlock::Variant::Variant(int data, const std::wstring& name)
    : mData(data), mName(name)
{
}

int TallGrassBlock::Variant::getData()
{
    return mData;
}

TallGrassBlock::Variant* TallGrassBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return VARIANTS[data];
}

std::wstring TallGrassBlock::Variant::toString() const
{
    return mName;
}

std::wstring TallGrassBlock::Variant::getSerializedName() const
{
    return mName;
}

void TallGrassBlock::blockStaticCtor()
{
    if (VARIANTS[0] == NULL)
    {
        VARIANTS[0] = new Variant(0, L"dead_bush");
        VARIANTS[1] = new Variant(1, L"grass");
        VARIANTS[2] = new Variant(2, L"fern");
        Variant::DEAD_BUSH = VARIANTS[0];
        Variant::GRASS = VARIANTS[1];
        Variant::FERN = VARIANTS[2];
        gTallGrass = Variant::GRASS;
        gFern = Variant::FERN;

        boost::unordered_set<Variant*> values;
        values.insert(VARIANTS[0]);
        values.insert(VARIANTS[1]);
        values.insert(VARIANTS[2]);
        VARIANT = EnumProperty<Variant*>::create(L"type", typeid(Variant), values, Predicates<Variant*>::alwaysTrue);
        TallGrassBlock_VARIANT = VARIANT;
    }
}

TallGrassBlock::TallGrassBlock()
    : Bush(Material::PLANT)
{
    DerivedInit();
    TypedBoxed<Variant*> variant(Variant::GRASS);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &variant));
    mIcons[0] = NULL;
    mIcons[1] = NULL;
    mIcons[2] = NULL;
}

TallGrassBlock::~TallGrassBlock()
{
}

bool TallGrassBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState* state, bool)
{
    return state->getValue(VARIANT) != Variant::DEAD_BUSH;
}

bool TallGrassBlock::isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*)
{
    return true;
}

void TallGrassBlock::performBonemeal(Level* level, Random*, const BlockPos& pos, const BlockState* state)
{
    DoublePlantBlock::Variant* variant = DoublePlantBlock::Variant::byData(2);
    if (state->getValue(VARIANT) == Variant::FERN)
    {
        variant = DoublePlantBlock::Variant::byData(3);
    }
    if (level->isEmptyBlock(pos.above()))
    {
        DoublePlantBlock* doublePlant = static_cast<DoublePlantBlock*>(Block::byId(175));
        doublePlant->placeAt(level, pos, variant, 2);
    }
}

const AABB* TallGrassBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return Bush::getShape(state, levelSource, pos);
}

bool TallGrassBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return Bush::canSurvive(level, pos, state);
}

bool TallGrassBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

int TallGrassBlock::getColor() const
{
    return GrassColor::get(0.5, 1.0);
}

int TallGrassBlock::getColor(const BlockState* state)
{
    if (state->getBlock() != this || state->getValue(VARIANT) == Variant::DEAD_BUSH)
    {
        return Block::getColor(state);
    }
    return GrassColor::get(0.5, 1.0);
}

int TallGrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int)
{
    return getColor(levelSource, pos, levelSource->getBlockState(pos));
}

int TallGrassBlock::getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    if (state->getBlock() != this || state->getValue(VARIANT) == Variant::DEAD_BUSH)
    {
        return BiomeColors::getAverageGrassColor(levelSource, pos);
    }
    return 0xFFFFFF;
}

Item* TallGrassBlock::getResource(const BlockState*, Random* random, int)
{
    return random->nextInt(8) == 0 ? Items::wheatSeeds : NULL;
}

int TallGrassBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return random->nextInt(lootBonus * 2 + 1) + 1;
}

void TallGrassBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                   const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                   not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        player->awardStat(Stats::blockMinedStats.data[Block::getId(this)],
                          GenericStats::param_blocksMined(state, 1));
        Variant* variant = state->getValue(VARIANT);
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Blocks::tallgrass, 1, variant->getData())));
        return;
    }
    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

not_null_ptr<ItemInstance> TallGrassBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, state->getBlock()->convertBlockStateToLegacyData(state)));
}

const BlockState* TallGrassBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &variant);
}

int TallGrassBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    return variant == NULL ? 0 : variant->getData();
}

unsigned int TallGrassBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351
        && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
        && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
    {
        return 0x9F5FDC40u;
    }
    return Block::GetInteractTooltip(tooltipData);
}

bool TallGrassBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* TallGrassBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockOffsetType TallGrassBlock::getOffsetType()
{
    return BLOCK_OFFSET_XYZ;
}

TextureAtlasSprite* TallGrassBlock::getTexture(const Direction*, const BlockState* state)
{
    return mIcons[state->getValue(VARIANT)->getData()];
}

void TallGrassBlock::registerIcons(IconRegister* iconRegister)
{
    mIcons[0] = iconRegister->registerIcon(L"deadbush");
    mIcons[1] = iconRegister->registerIcon(L"tallgrass");
    mIcons[2] = iconRegister->registerIcon(L"fern");
}
