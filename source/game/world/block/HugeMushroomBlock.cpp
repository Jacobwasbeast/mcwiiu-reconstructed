#include "game/world/block/HugeMushroomBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static HugeMushroomBlock::Variant gHugeMushroomVariants[13] = {
    HugeMushroomBlock::Variant(0, L"all_inside"),
    HugeMushroomBlock::Variant(1, L"north_west"),
    HugeMushroomBlock::Variant(2, L"north"),
    HugeMushroomBlock::Variant(3, L"north_east"),
    HugeMushroomBlock::Variant(4, L"west"),
    HugeMushroomBlock::Variant(5, L"center"),
    HugeMushroomBlock::Variant(6, L"east"),
    HugeMushroomBlock::Variant(7, L"south_west"),
    HugeMushroomBlock::Variant(8, L"south"),
    HugeMushroomBlock::Variant(9, L"south_east"),
    HugeMushroomBlock::Variant(10, L"stem"),
    HugeMushroomBlock::Variant(11, L"all_outside"),
    HugeMushroomBlock::Variant(12, L"all_stem")
};

static HugeMushroomBlock::Variant* gHugeMushroomVariantPointers[13];
static EnumProperty<HugeMushroomBlock::Variant*>* gHugeMushroomVariantProperty = NULL;

int getMushroomDataValue(HugeMushroomBlock::Variant* variant)
{
    TypedBoxed<HugeMushroomBlock::Variant*> value(variant);
    Block* block = Blocks::red_mushroom_block;
    return block->convertBlockStateToLegacyData(
        block->defaultBlockState()->setBoxedValue(gHugeMushroomVariantProperty, &value));
}

HugeMushroomBlock::Variant::Variant(int data, const std::wstring& name)
    : mData(data), mName(name)
{
}

int HugeMushroomBlock::Variant::getData()
{
    return mData;
}

HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::byData(int data)
{
    if (data < 0 || data > 12)
    {
        data = 0;
    }
    return gHugeMushroomVariantPointers[data];
}

std::wstring HugeMushroomBlock::Variant::toString() const
{
    return mName;
}

std::wstring HugeMushroomBlock::Variant::getSerializedName() const
{
    return mName;
}

void HugeMushroomBlock::blockStaticCtor()
{
    for (int i = 0; i < 13; ++i)
    {
        gHugeMushroomVariantPointers[i] = &gHugeMushroomVariants[i];
    }

    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 13; ++i)
    {
        values.insert(gHugeMushroomVariantPointers[i]);
    }
    gHugeMushroomVariantProperty = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

HugeMushroomBlock::HugeMushroomBlock(Material* material, const MaterialColor* materialColor, Block* otherBlock)
    : Block(material, materialColor), mOtherBlock(otherBlock), mIsOtherBlock(0), mStemTexture(NULL), mInsideTexture(NULL)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::byData(0));
    registerDefaultState(defaultBlockState()->setBoxedValue(gHugeMushroomVariantProperty, &value));
    mIsOtherBlock = -1;
    mTextures[0] = NULL;
    mTextures[1] = NULL;
}

HugeMushroomBlock::~HugeMushroomBlock()
{
}

int HugeMushroomBlock::getResourceCount(Random* random)
{
    int count = random->nextInt(10) - 7;
    return count < 0 ? 0 : count;
}

const MaterialColor* HugeMushroomBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    int data = state->getValue(gHugeMushroomVariantProperty)->getData();
    if (data == 12)
    {
        return MaterialColor::COLOR_BROWN;
    }
    if (data != 0 && data != 10)
    {
        return Block::getMapColor(state, levelSource, pos);
    }
    return MaterialColor::COLOR_RED;
}

Item* HugeMushroomBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(mOtherBlock);
}

not_null_ptr<ItemInstance> HugeMushroomBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(gHugeMushroomVariantProperty)->getData()));
}

const BlockState* HugeMushroomBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int auxValue, boost::shared_ptr<LivingEntity>)
{
    return getBlockState(auxValue);
}

const BlockState* HugeMushroomBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(gHugeMushroomVariantProperty, &value);
}

int HugeMushroomBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gHugeMushroomVariantProperty)->getData();
}

const BlockState* HugeMushroomBlock::rotate(const BlockState* state, Rotation* rotation)
{
    static const int clockwise[13] = { 0, 7, 4, 1, 8, 5, 2, 9, 6, 3, 10, 11, 12 };
    static const int clockwise180[13] = { 0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 10, 11, 12 };
    static const int counterClockwise[13] = { 0, 3, 6, 9, 2, 5, 8, 1, 4, 7, 10, 11, 12 };
    int data = state->getValue(gHugeMushroomVariantProperty)->getData();
    const int* mapping = NULL;
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90: mapping = clockwise; break;
    case Rotation::eTYPE_Clockwise180: mapping = clockwise180; break;
    case Rotation::eTYPE_CounterClockwise90: mapping = counterClockwise; break;
    default: return state;
    }
    TypedBoxed<Variant*> value(Variant::byData(mapping[data]));
    return state->setBoxedValue(gHugeMushroomVariantProperty, &value);
}

const BlockState* HugeMushroomBlock::mirror(const BlockState* state, Mirror* mirror)
{
    int data = state->getValue(gHugeMushroomVariantProperty)->getData();
    static const int leftRight[13] = { 0, 3, 2, 1, 6, 5, 4, 9, 8, 7, 10, 11, 12 };
    static const int frontBack[13] = { 0, 7, 8, 9, 4, 5, 6, 1, 2, 3, 10, 11, 12 };
    const int* mapping = mirror->getType() == Mirror::eTYPE_LeftRight ? leftRight : frontBack;
    if (mirror->getType() == Mirror::eTYPE_None)
    {
        return state;
    }
    TypedBoxed<Variant*> value(Variant::byData(mapping[data]));
    return state->setBoxedValue(gHugeMushroomVariantProperty, &value);
}

not_null_ptr<ItemInstance> HugeMushroomBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(gHugeMushroomVariantProperty)->getData()));
}

BlockStateDefinition* HugeMushroomBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gHugeMushroomVariantProperty };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* HugeMushroomBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int data = state->getValue(gHugeMushroomVariantProperty)->getData();
    if (data == 10 || data == 12)
    {
        return mStemTexture;
    }
    if (data == 11)
    {
        return mInsideTexture;
    }
    if (data == 1 || data == 2 || data == 3 || data == 4 || data == 5 || data == 6 || data == 7 || data == 8 || data == 9)
    {
        return mTextures[direction->get3DDataValue() == 1 ? 0 : 1];
    }
    return mInsideTexture;
}

void HugeMushroomBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    mTextures[0] = iconRegister->registerIcon(base + L"_skin");
    mTextures[1] = iconRegister->registerIcon(base + L"_skin_top");
    mStemTexture = iconRegister->registerIcon(base + L"_stem");
    mInsideTexture = iconRegister->registerIcon(base + L"_inside");
}
