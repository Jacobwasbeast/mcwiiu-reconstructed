#include "game/world/block/DirtBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/DyeColor.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/SeaGrass.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

namespace
{
const int kFirstHoeItemId = 0x122;
const int kLastHoeItemId = 0x126;
const int kDyeItemId = 0x15f;
const int kWaterDepth = 0x10e;
const unsigned int kHoeTooltip = IDS_TOOLTIPS_TILL;
const unsigned int kDyeTooltip = IDS_TOOLTIPS_GROW;
}

DirtBlock::Variant* DirtBlock::Variant::DIRT = NULL;
DirtBlock::Variant* DirtBlock::Variant::COARSE_DIRT = NULL;
DirtBlock::Variant* DirtBlock::Variant::PODZOL = NULL;
EnumProperty<DirtBlock::Variant*>* DirtBlock::VARIANT = NULL;
static DirtBlock::Variant* gVariantsByData[3] = { NULL, NULL, NULL };
static AbstractProperty<bool>* gSnowyProperty =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"snowy"));

DirtBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                            const MaterialColor* materialColor, const std::wstring& texture,
                            const std::wstring& topTexture, const std::wstring& sideTexture)
    : mData(data), mName(name), mNameId(nameId), mDescriptionId(descriptionId),
      mMaterialColor(materialColor), mTexture(texture), mTopTexture(topTexture),
      mSideTexture(sideTexture), mIcon(NULL), mSideIcon(NULL), mTopIcon(NULL)
{
}

const MaterialColor* DirtBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

int DirtBlock::Variant::getData()
{
    return mData;
}

DirtBlock::Variant* DirtBlock::Variant::byData(int data)
{
    if (data > 2)
    {
        data = 0;
    }

    return gVariantsByData[data];
}

TextureAtlasSprite* DirtBlock::Variant::getTexture(const Direction* direction)
{
    if (mTopIcon != NULL && direction == Direction::UP)
    {
        return mTopIcon;
    }
    if (mSideIcon != NULL && direction != Direction::UP && direction != Direction::DOWN)
    {
        return mSideIcon;
    }
    return mIcon;
}

void DirtBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTexture);
    if (!mSideTexture.empty())
    {
        mSideIcon = iconRegister->registerIcon(mSideTexture);
    }
    if (!mTopTexture.empty())
    {
        mTopIcon = iconRegister->registerIcon(mTopTexture);
    }
}

int DirtBlock::Variant::getNameId()
{
    return mNameId;
}

int DirtBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

std::wstring DirtBlock::Variant::toString() const
{
    return mName;
}

std::wstring DirtBlock::Variant::getSerializedName() const
{
    return mName;
}

DirtBlock::DirtBlock()
    : Block(Material::DIRT)
{
    DerivedInit();
    TypedBoxed<Variant*> variant(Variant::DIRT);
    TypedBoxed<bool> snowy(false);
    const BlockState* state = mStateDefinition->any()->setBoxedValue(VARIANT, &variant);
    registerDefaultState(state->setBoxedValue(gSnowyProperty, &snowy));
}

bool DirtBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool DirtBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos,
                                  const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void DirtBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos,
                                const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

void DirtBlock::blockStaticCtor()
{
    if (Variant::DIRT != NULL)
    {
        return;
    }

    Variant::DIRT = new Variant(0, L"dirt", IDS_TILE_DIRT, IDS_DESC_DIRT, MaterialColor::DIRT,
                                L"dirt", L"", L"");
    Variant::COARSE_DIRT = new Variant(1, L"coarse_dirt", IDS_TILE_DIRT_COARSE, IDS_DESC_DIRT_COARSE, MaterialColor::DIRT,
                                       L"coarse_dirt", L"", L"");
    Variant::PODZOL = new Variant(2, L"podzol", IDS_TILE_DIRT_PODZOL, IDS_DESC_DIRT_PODZOL, MaterialColor::PODZOL,
                                  L"dirt", L"dirt_podzol_top", L"dirt_podzol_side");

    gVariantsByData[0] = Variant::DIRT;
    gVariantsByData[1] = Variant::COARSE_DIRT;
    gVariantsByData[2] = Variant::PODZOL;

    boost::unordered_set<Variant*> variants;
    variants.insert(Variant::DIRT);
    variants.insert(Variant::COARSE_DIRT);
    variants.insert(Variant::PODZOL);
    VARIANT = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant), variants, Predicates<Variant*>::alwaysTrue);
}

const MaterialColor* DirtBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                            const BlockPos& pos)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

const BlockState* DirtBlock::fillVirtualBlockStateProperties(const BlockState* state,
                                                              LevelSource* levelSource,
                                                              const BlockPos& pos)
{
    if (state->getValue(VARIANT) == Variant::PODZOL)
    {
        Block* above = levelSource->getBlockState(pos.above())->getBlock();
        TypedBoxed<bool> snowy(above == Blocks::snow || above == Blocks::snow_layer);
        return state->setBoxedValue(gSnowyProperty, &snowy);
    }
    return state;
}

not_null_ptr<ItemInstance> DirtBlock::getCloneItemInstance(Level*, const BlockPos&,
                                                           const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, convertBlockStateToLegacyData(state)));
}

const BlockState* DirtBlock::getBlockState(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    static const BlockState* states[3] = { NULL, NULL, NULL };
    if (states[0] == NULL)
    {
        for (int i = 0; i < 3; ++i)
        {
            TypedBoxed<Variant*> value(Variant::byData(i));
            states[i] = defaultBlockState()->setBoxedValue(VARIANT, &value);
        }
    }
    return states[data];
}

int DirtBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

unsigned int DirtBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    if (itemId >= kFirstHoeItemId && itemId <= kLastHoeItemId)
    {
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
            && tooltipData.m_bCanPlace
            && tooltipData.m_pBlockState->getValue(VARIANT) != Variant::PODZOL)
        {
            return kHoeTooltip;
        }
    }
    else if (itemId == kDyeItemId)
    {
        BlockPos above = tooltipData.m_blockPos.above();
        if (((Level*)tooltipData.m_pLevel)->isUnderWater(above, kWaterDepth, 0)
            && tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return kDyeTooltip;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* DirtBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT, gSnowyProperty };
    return new BlockStateDefinition(this, properties, 2);
}

int DirtBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    return (variant == Variant::PODZOL ? Variant::COARSE_DIRT : variant)->getData();
}

TextureAtlasSprite* DirtBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return state->getValue(VARIANT)->getTexture(direction);
}

void DirtBlock::registerIcons(IconRegister* iconRegister)
{
    blockStaticCtor();
    Variant::DIRT->registerIcons(iconRegister);
    Variant::COARSE_DIRT->registerIcons(iconRegister);
    Variant::PODZOL->registerIcons(iconRegister);
}

DirtBlock::~DirtBlock()
{
}
