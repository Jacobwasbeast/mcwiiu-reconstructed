#include "game/world/block/SaplingBlock.hpp"
#include <typeinfo>
#include <boost/unordered_set.hpp>
#include "game/core/DyeColor.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/BirchFeature.hpp"
#include "game/world/generation/MegaJungleTreeFeature.hpp"
#include "game/world/generation/RoofTreeFeature.hpp"
#include "game/world/generation/SavannaTreeFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"

static EnumProperty<PlanksBlock::Variant*>* gSaplingVariant = NULL;
static AbstractProperty<bool>* gSaplingStage = NULL;
static const unsigned int SAPLING_DESCRIPTION_IDS[4] = { 0xE020DDDBu, 0xF9DC3BECu, 0xC8AE199Cu, 0x6793A4E9u };

class SaplingVariantPredicate : public Predicate<PlanksBlock::Variant*>
{
public:
    virtual bool apply(PlanksBlock::Variant* value) const { return value != NULL; }
};

SaplingBlock::SaplingBlock()
{
    blockStaticCtor();
    DerivedInit();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(0));
    TypedBoxed<bool> stage(false);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gSaplingVariant, &variant)->setBoxedValue(gSaplingStage, &stage));
    for (int i = 0; i < 6; ++i) mIcons[i] = NULL;
}

SaplingBlock::~SaplingBlock() {}

bool SaplingBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool) { return true; }

bool SaplingBlock::isBonemealSuccess(Level*, Random* random, const BlockPos&, const BlockState*)
{
    return ((GameMode*)Minecraft::GetInstance()->mGameMode)->isCreative() || random->nextFloat() < 0.45f;
}

bool SaplingBlock::isSapling(Level* level, const BlockPos& pos, PlanksBlock::Variant* variant)
{
    const BlockState* state = level->getBlockState(pos);
    return state->getBlock() == this && state->getValue(gSaplingVariant) == variant;
}

bool SaplingBlock::isTwoByTwoSaplingOfType(Level* level, const BlockPos& pos, int x, int z, PlanksBlock::Variant* variant)
{
    return isSapling(level, pos.offset(x, 0, z), variant)
        && isSapling(level, pos.offset(x + 1, 0, z), variant)
        && isSapling(level, pos.offset(x, 0, z + 1), variant)
        && isSapling(level, pos.offset(x + 1, 0, z + 1), variant);
}

void SaplingBlock::growTree(Level* level, const BlockPos& pos, const BlockState* state, Random* random, bool)
{
    int data = state->getValue(gSaplingVariant)->getData();
    AbstractTreeFeature* feature;
    if (data == 0) feature = random->nextInt(10) == 0 ? static_cast<AbstractTreeFeature*>(new BigTreeFeature(false)) : new TreeFeature(false);
    else if (data == 1) feature = new SpruceFeature(false);
    else if (data == 2) feature = new BirchFeature(false, false);
    else if (data == 3) feature = new MegaJungleTreeFeature(false, 10, 20, Blocks::log->getBlockState(3), Blocks::leaves->getBlockState(3));
    else if (data == 4) feature = new SavannaTreeFeature(false);
    else feature = new RoofTreeFeature(false);
    level->setBlock(pos, Blocks::AIR->defaultBlockState(), 4, false);
    if (!feature->place(level, *random, pos)) level->setBlock(pos, state, 4, false);
    delete feature;
}

void SaplingBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    if (((GameMode*)Minecraft::GetInstance()->mGameMode)->isCreative()) growTree(level, pos, state, random, true);
    else advanceTree(level, pos, state, random);
}

void SaplingBlock::blockStaticCtor()
{
    if (gSaplingVariant != NULL) return;
    boost::unordered_set<PlanksBlock::Variant*> values;
    for (int i = 0; i < 6; ++i) values.insert(PlanksBlock::Variant::byData(i));
    SaplingVariantPredicate predicate;
    gSaplingVariant = EnumProperty<PlanksBlock::Variant*>::create(L"type", typeid(PlanksBlock::Variant), values, &predicate);
    gSaplingStage = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"stage"));
}

std::wstring SaplingBlock::getName() { return std::wstring(L10N::GetString(0x6F4DDFC1)); }
const AABB* SaplingBlock::getShape(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }

void SaplingBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Bush::tick(level, pos, state, random);
    if (!level->mIsClientSide && level->getRawBrightness(pos.above()) > 8 && random->nextInt(7) == 0) advanceTree(level, pos, state, random);
}

void SaplingBlock::advanceTree(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (state->getValue(gSaplingStage)) growTree(level, pos, state, random, false);
    else { TypedBoxed<bool> stage(true); level->setBlock(pos, state->setBoxedValue(gSaplingStage, &stage), 4, false); }
}

int SaplingBlock::getSpawnResourcesAuxValue(const BlockState* state) { return state->getValue(gSaplingVariant)->getData(); }

const BlockState* SaplingBlock::getBlockState(int data)
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(data & 7));
    TypedBoxed<bool> stage((data & 8) != 0);
    return defaultBlockState()->setBoxedValue(gSaplingVariant, &variant)->setBoxedValue(gSaplingStage, &stage);
}

int SaplingBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gSaplingVariant)->getData() | (state->getValue(gSaplingStage) ? 8 : 0);
}

unsigned int SaplingBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    ItemInstance* item = tooltipData.m_pItemInstance.get();
    if (item != NULL && Item::getId(item->getItem()) == 0x15F && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail && DyeColor::byItemData(item->getAuxValue())->getBlockData() == 0) return 0x9F5FDC40u;
    return Block::GetInteractTooltip(tooltipData);
}

bool SaplingBlock::canContainLiquid(const BlockState*) const { return false; }

BlockStateDefinition* SaplingBlock::createBlockStateDefinition()
{
    blockStaticCtor();
    const Property* properties[] = { gSaplingVariant, gSaplingStage };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* SaplingBlock::getTexture(const Direction*, const BlockState* state) { return mIcons[state->getValue(gSaplingVariant)->getData()]; }

void SaplingBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 6; ++i) mIcons[i] = iconRegister->registerIcon(getIconName() + L"_" + PlanksBlock::Variant::byData(i)->getSerializedName());
}

int SaplingBlock::getDescriptionId(int auxValue)
{
    int data = getBlockState(auxValue)->getValue(gSaplingVariant)->getData();
    return SAPLING_DESCRIPTION_IDS[data < 0 ? 0 : data];
}
