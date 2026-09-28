#include "game/world/block/SandBlock.hpp"
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
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/ItemId.hpp"

static SandBlock::Variant* gSandVariants[2] = { NULL, NULL };
static AbstractProperty<SandBlock::Variant*>* gSandVariantProperty = NULL;

SandBlock::Variant* SandBlock::Variant::SAND = NULL;
SandBlock::Variant* SandBlock::Variant::RED_SAND = NULL;
AbstractProperty<SandBlock::Variant*>* SandBlock::TYPE = NULL;

SandBlock::Variant::Variant(int data, const std::wstring& name, int nameId, int descriptionId,
                            const MaterialColor* materialColor, int dustColor)
    : mData(data), mName(name), mNameId(nameId), mDescriptionId(descriptionId),
      mMaterialColor(materialColor), mDustColor(dustColor)
{
}

const MaterialColor* SandBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

int SandBlock::Variant::getData()
{
    return mData;
}

SandBlock::Variant* SandBlock::Variant::byData(int data)
{
    if (data > 1)
    {
        data = 0;
    }
    return gSandVariants[data];
}

int SandBlock::Variant::getDustColor()
{
    return mDustColor;
}

std::wstring SandBlock::Variant::getName()
{
    return mName;
}

std::wstring SandBlock::Variant::toString() const
{
    return mName;
}

std::wstring SandBlock::Variant::getSerializedName() const
{
    return mName;
}

int SandBlock::Variant::getNameId()
{
    return mNameId;
}

int SandBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

SandBlock::SandBlock()
    : FallingBlock(Material::SAND)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::SAND);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(TYPE, &value));
    mTextures[0] = NULL;
    mTextures[1] = NULL;
}

bool SandBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool SandBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos,
                                  const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void SandBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos,
                                const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

void SandBlock::blockStaticCtor()
{
    if (gSandVariants[0] != NULL)
    {
        return;
    }

    gSandVariants[0] = new Variant(0, L"sand", IDS_TILE_SAND, IDS_DESC_SAND, MaterialColor::SAND,
                                   -2370656);
    gSandVariants[1] = new Variant(1, L"red_sand", IDS_TILE_SAND_RED, IDS_DESC_SAND_RED,
                                   MaterialColor::COLOR_ORANGE, -5679071);
    Variant::SAND = gSandVariants[0];
    Variant::RED_SAND = gSandVariants[1];

    boost::unordered_set<Variant*> variants;
    variants.insert(gSandVariants[0]);
    variants.insert(gSandVariants[1]);
    gSandVariantProperty = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), variants, Predicates<Variant*>::alwaysTrue);
    TYPE = gSandVariantProperty;
}

int SandBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const MaterialColor* SandBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(TYPE)->getMaterialColor();
}

const BlockState* SandBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(TYPE, &value);
}

int SandBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

unsigned int SandBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::DYE)
    {
        BlockPos above = tooltipData.m_blockPos.above();
        if (((Level*)tooltipData.m_pLevel)->isUnderWater(above, 0x10E, 0)
            && tooltipData.m_eInteractionResult != 2
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return IDS_TOOLTIPS_GROW;
        }
    }
    return Block::GetInteractTooltip(tooltipData);
}

BlockStateDefinition* SandBlock::createBlockStateDefinition()
{
    const Property* properties[1];
    properties[0] = TYPE;
    return new BlockStateDefinition(this, properties, 1);
}

unsigned int SandBlock::getDustColor(const BlockState* state)
{
    return state->getValue(TYPE)->getDustColor();
}

TextureAtlasSprite* SandBlock::getTexture(const Direction*, const BlockState* state)
{
    return mTextures[state->getValue(TYPE)->getData()];
}

void SandBlock::registerIcons(IconRegister* iconRegister)
{
    mTextures[0] = iconRegister->registerIcon(Variant::SAND->getName());
    mTextures[1] = iconRegister->registerIcon(Variant::RED_SAND->getName());
}

SandBlock::~SandBlock()
{
}
