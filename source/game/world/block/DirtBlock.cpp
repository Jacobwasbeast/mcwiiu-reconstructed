#include "game/world/block/DirtBlock.hpp"

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
const unsigned int kHoeTooltip = 0x70889480u;
const unsigned int kDyeTooltip = 0x9f5fdc40u;
}

DirtBlock::Variant* gVariants[3] = { NULL, NULL, NULL };
EnumProperty<DirtBlock::Variant*>* gVariantProperty = NULL;
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
    if (data < 0 || data > 2)
    {
        data = 0;
    }

    return gVariants[data];
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
    TypedBoxed<Variant*> variant(gVariants[0]);
    TypedBoxed<bool> snowy(false);
    const BlockState* state = mStateDefinition->any()->setBoxedValue(gVariantProperty, &variant);
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
    if (gVariants[0] != NULL)
    {
        return;
    }

    gVariants[0] = new Variant(0, L"dirt", 0x672CD46C, 0x5437FAF1, MaterialColor::DIRT,
                               L"dirt", L"", L"");
    gVariants[1] = new Variant(1, L"coarse_dirt", 0xC79DB172, 0x935970C7, MaterialColor::DIRT,
                               L"coarse_dirt", L"", L"");
    gVariants[2] = new Variant(2, L"podzol", 0x64186D51, 0x8FB42134, MaterialColor::PODZOL,
                               L"dirt", L"dirt_podzol_top", L"dirt_podzol_side");

    boost::unordered_set<Variant*> variants;
    variants.insert(gVariants[0]);
    variants.insert(gVariants[1]);
    variants.insert(gVariants[2]);
    gVariantProperty = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant), variants, Predicates<Variant*>::alwaysTrue);
}

const MaterialColor* DirtBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                            const BlockPos& pos)
{
    return state->getValue(gVariantProperty)->getMaterialColor();
}

const BlockState* DirtBlock::fillVirtualBlockStateProperties(const BlockState* state,
                                                              LevelSource* levelSource,
                                                              const BlockPos& pos)
{
    if (state->getValue(gVariantProperty) == gVariants[2])
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
            states[i] = defaultBlockState()->setBoxedValue(gVariantProperty, &value);
        }
    }
    return states[data];
}

int DirtBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gVariantProperty)->getData();
}

unsigned int DirtBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    if (itemId >= kFirstHoeItemId && itemId <= kLastHoeItemId)
    {
        if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Success
            && tooltipData.m_bIsInside
            && tooltipData.m_pBlockState->getValue(gVariantProperty) != gVariants[2])
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
    const Property* properties[] = { gVariantProperty, gSnowyProperty };
    return new BlockStateDefinition(this, properties, 2);
}

int DirtBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    Variant* variant = state->getValue(gVariantProperty);
    return (variant == gVariants[2] ? gVariants[1] : variant)->getData();
}

TextureAtlasSprite* DirtBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return state->getValue(gVariantProperty)->getTexture(direction);
}

void DirtBlock::registerIcons(IconRegister* iconRegister)
{
    blockStaticCtor();
    gVariants[0]->registerIcons(iconRegister);
    gVariants[1]->registerIcons(iconRegister);
    gVariants[2]->registerIcons(iconRegister);
}

DirtBlock::~DirtBlock()
{
}
