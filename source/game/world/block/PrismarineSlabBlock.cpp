#include "game/world/block/PrismarineSlabBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/core/Direction.hpp"

PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::ROUGH = NULL;
PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::BRICKS = NULL;
PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::DARK = NULL;
static PrismarineSlabBlock::Variant* gPrismarineSlabVariants[3] = { NULL, NULL, NULL };
EnumProperty<PrismarineSlabBlock::Variant*>* PrismarineSlabBlock::VARIANT = NULL;

PrismarineSlabBlock::Variant::Variant(int data, const std::wstring& name, int nameId,
                                      int descriptionId, const std::wstring& serializedName)
    : mData(data),
      mName(name),
      mNameId(nameId),
      mDescriptionId(descriptionId),
      mSerializedName(serializedName),
      mTexture(NULL)
{
}

int PrismarineSlabBlock::Variant::getData()
{
    return mData;
}

PrismarineSlabBlock::Variant* PrismarineSlabBlock::Variant::byData(int data)
{
    if (data > 2)
    {
        data = 0;
    }
    return gPrismarineSlabVariants[data];
}

int PrismarineSlabBlock::Variant::getNameId()
{
    return mNameId;
}

void PrismarineSlabBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mTexture = iconRegister->registerIcon(mSerializedName);
}

std::wstring PrismarineSlabBlock::Variant::toString() const
{
    return mName;
}

std::wstring PrismarineSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

void PrismarineSlabBlock::blockStaticCtor()
{
    Variant::ROUGH = new Variant(0, L"prismarine_slab", IDS_TILE_SLAB_PRISMARINE, IDS_DESC_SLAB_PRISMARINE, L"prismarine_rough");
    Variant::BRICKS = new Variant(1, L"prismarine_bricks_slab", IDS_TILE_SLAB_PRISMARINE_BRICKS, IDS_DESC_SLAB_PRISMARINE, L"prismarine_bricks");
    Variant::DARK = new Variant(2, L"dark_prismarine_slab", IDS_TILE_SLAB_DARK_PRISMARINE, IDS_DESC_SLAB_PRISMARINE, L"prismarine_dark");
    gPrismarineSlabVariants[0] = Variant::ROUGH;
    gPrismarineSlabVariants[1] = Variant::BRICKS;
    gPrismarineSlabVariants[2] = Variant::DARK;

    boost::unordered_set<Variant*> values;
    values.insert(gPrismarineSlabVariants[0]);
    values.insert(gPrismarineSlabVariants[1]);
    values.insert(gPrismarineSlabVariants[2]);
    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant*), values,
                                              Predicates<Variant*>::alwaysTrue);
}

PrismarineSlabBlock::PrismarineSlabBlock()
    : HalfSlabBlock(Material::STONE)
{
}

void PrismarineSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    if (!isFullSize())
    {
        TypedBoxed<Half*> half(BOTTOM);
        state = state->setBoxedValue(TYPE, &half);
    }
    TypedBoxed<Variant*> variant(Variant::ROUGH);
    registerDefaultState(state->setBoxedValue(VARIANT, &variant));
}

Item* PrismarineSlabBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::prismarine_slab);
}

const MaterialColor* PrismarineSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT) == Variant::ROUGH ? MaterialColor::COLOR_CYAN : MaterialColor::DIAMOND;
}

int PrismarineSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

not_null_ptr<ItemInstance> PrismarineSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::prismarine_slab, 1,
                                                       state->getValue(VARIANT)->getData()));
}

const BlockState* PrismarineSlabBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data & 7));
    const BlockState* state = defaultBlockState()->setBoxedValue(VARIANT, &variant);
    if (!isFullSize())
    {
        TypedBoxed<Half*> half((data & 8) == 0 ? BOTTOM : TOP);
        state = state->setBoxedValue(TYPE, &half);
    }
    return state;
}

int PrismarineSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    if (!isFullSize() && state->getValue(TYPE) == TOP)
    {
        data |= 8;
    }
    return data;
}

int PrismarineSlabBlock::getAuxName(int auxValue)
{
    return Variant::byData(auxValue)->getNameId();
}

Property* PrismarineSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* PrismarineSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

BlockStateDefinition* PrismarineSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        const Property* properties[] = { VARIANT };
        return new BlockStateDefinition(this, properties, 1);
    }

    const Property* properties[] = { TYPE, VARIANT };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* PrismarineSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return Blocks::prismarine->getTexture(direction,
        Blocks::prismarine->getBlockState(state->getValue(VARIANT)->getData()));
}

void PrismarineSlabBlock::registerIcons(IconRegister* iconRegister)
{
    gPrismarineSlabVariants[0]->registerIcons(iconRegister);
    gPrismarineSlabVariants[1]->registerIcons(iconRegister);
    gPrismarineSlabVariants[2]->registerIcons(iconRegister);
}
