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

HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::NORTH_WEST =
    new HugeMushroomBlock::Variant(1, L"north_west");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::NORTH =
    new HugeMushroomBlock::Variant(2, L"north");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::NORTH_EAST =
    new HugeMushroomBlock::Variant(3, L"north_east");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::WEST =
    new HugeMushroomBlock::Variant(4, L"west");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::CENTER =
    new HugeMushroomBlock::Variant(5, L"center");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::EAST =
    new HugeMushroomBlock::Variant(6, L"east");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::SOUTH_WEST =
    new HugeMushroomBlock::Variant(7, L"south_west");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::SOUTH =
    new HugeMushroomBlock::Variant(8, L"south");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::SOUTH_EAST =
    new HugeMushroomBlock::Variant(9, L"south_east");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::STEM =
    new HugeMushroomBlock::Variant(10, L"stem");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::ALL_INSIDE =
    new HugeMushroomBlock::Variant(0, L"all_inside");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::ALL_OUTSIDE =
    new HugeMushroomBlock::Variant(11, L"all_outside");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::ALL_STEM =
    new HugeMushroomBlock::Variant(12, L"all_stem");
HugeMushroomBlock::Variant* HugeMushroomBlock::Variant::VALUES[13] = {
    HugeMushroomBlock::Variant::ALL_INSIDE,
    HugeMushroomBlock::Variant::NORTH_WEST,
    HugeMushroomBlock::Variant::NORTH,
    HugeMushroomBlock::Variant::NORTH_EAST,
    HugeMushroomBlock::Variant::WEST,
    HugeMushroomBlock::Variant::CENTER,
    HugeMushroomBlock::Variant::EAST,
    HugeMushroomBlock::Variant::SOUTH_WEST,
    HugeMushroomBlock::Variant::SOUTH,
    HugeMushroomBlock::Variant::SOUTH_EAST,
    HugeMushroomBlock::Variant::STEM,
    HugeMushroomBlock::Variant::ALL_OUTSIDE,
    HugeMushroomBlock::Variant::ALL_STEM
};
EnumProperty<HugeMushroomBlock::Variant*>* HugeMushroomBlock::VARIANT = NULL;

int getMushroomDataValue(HugeMushroomBlock::Variant* variant)
{
    TypedBoxed<HugeMushroomBlock::Variant*> value(variant);
    Block* block = Blocks::red_mushroom_block;
    return block->convertBlockStateToLegacyData(
        block->defaultBlockState()->setBoxedValue(HugeMushroomBlock::VARIANT, &value));
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
    for (int i = 0; i < 13; ++i)
    {
        if (VALUES[i]->getData() == data)
        {
            return VALUES[i];
        }
    }
    return NULL;
}

std::wstring HugeMushroomBlock::Variant::toString() const
{
    return mName;
}

std::wstring HugeMushroomBlock::Variant::getSerializedName() const
{
    return mName;
}

template std::wstring _toString<HugeMushroomBlock::Variant>(HugeMushroomBlock::Variant*);

void HugeMushroomBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 13; ++i)
    {
        values.insert(Variant::VALUES[i]);
    }
    VARIANT = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

HugeMushroomBlock::HugeMushroomBlock(Material* material, const MaterialColor* materialColor, Block* otherBlock)
    : Block(material, materialColor), mOtherBlock(otherBlock), mIsOtherBlock(0), mStemTexture(NULL), mInsideTexture(NULL)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::ALL_OUTSIDE);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &value));
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
    int data = state->getValue(VARIANT)->getData();
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
        state->getValue(VARIANT)->getData()));
}

const BlockState* HugeMushroomBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int auxValue, boost::shared_ptr<LivingEntity>)
{
    return getBlockState(auxValue);
}

const BlockState* HugeMushroomBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}

int HugeMushroomBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* HugeMushroomBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(VARIANT)->getData())
        {
        case 10:
            break;
        case 1:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 2:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 3:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 4:
        {
            TypedBoxed<Variant*> value(Variant::EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 6:
        {
            TypedBoxed<Variant*> value(Variant::WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 7:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 8:
        {
            TypedBoxed<Variant*> value(Variant::NORTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 9:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        default:
            return state;
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(VARIANT)->getData())
        {
        case 1:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 2:
        {
            TypedBoxed<Variant*> value(Variant::WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 3:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 4:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 6:
        {
            TypedBoxed<Variant*> value(Variant::NORTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 7:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 8:
        {
            TypedBoxed<Variant*> value(Variant::EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 9:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        default:
            return state;
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(VARIANT)->getData())
        {
        case 1:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 2:
        {
            TypedBoxed<Variant*> value(Variant::EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 3:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 4:
        {
            TypedBoxed<Variant*> value(Variant::NORTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 6:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 7:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 8:
        {
            TypedBoxed<Variant*> value(Variant::WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 9:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        default:
            return state;
        }
    default:
        return state;
    }
}

const BlockState* HugeMushroomBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        switch (state->getValue(VARIANT)->getData())
        {
        case 1:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 2:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 3:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 7:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 8:
        {
            TypedBoxed<Variant*> value(Variant::NORTH);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 9:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        default:
            return Block::mirror(state, mirror);
        }
    case Mirror::eTYPE_FrontBack:
        switch (state->getValue(VARIANT)->getData())
        {
        case 1:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 3:
        {
            TypedBoxed<Variant*> value(Variant::NORTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 4:
        {
            TypedBoxed<Variant*> value(Variant::EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 6:
        {
            TypedBoxed<Variant*> value(Variant::WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 7:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_EAST);
            return state->setBoxedValue(VARIANT, &value);
        }
        case 9:
        {
            TypedBoxed<Variant*> value(Variant::SOUTH_WEST);
            return state->setBoxedValue(VARIANT, &value);
        }
        default:
            return Block::mirror(state, mirror);
        }
    default:
        return Block::mirror(state, mirror);
    }
}

not_null_ptr<ItemInstance> HugeMushroomBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(VARIANT)->getData()));
}

BlockStateDefinition* HugeMushroomBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* HugeMushroomBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    if (data == 10 && direction->get3DDataValue() > 1)
        return mStemTexture;

    if (mIsOtherBlock == -1)
        mIsOtherBlock = mOtherBlock == Blocks::red_mushroom;

    if ((data >= 1 && data <= 9 && direction == Direction::UP) ||
        (data >= 1 && data <= 3 && direction == Direction::NORTH) ||
        (data >= 7 && data <= 9 && direction == Direction::SOUTH) ||
        ((data == 1 || data == 4 || data == 7) && direction == Direction::WEST) ||
        ((data == 3 || data == 6 || data == 9) && direction == Direction::EAST) ||
        data == 11)
        return mTextures[mIsOtherBlock];

    if (data == 12)
        return mStemTexture;
    return mInsideTexture;
}

void HugeMushroomBlock::registerIcons(IconRegister* iconRegister)
{
    static const std::wstring skinNames[2] = { L"skin_brown", L"skin_red" };
    for (int i = 0; i < 2; ++i)
        mTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + skinNames[i]);
    mInsideTexture = iconRegister->registerIcon(getIconName() + L"_" + L"inside");
    mStemTexture = iconRegister->registerIcon(getIconName() + L"_" + L"skin_stem");
}
