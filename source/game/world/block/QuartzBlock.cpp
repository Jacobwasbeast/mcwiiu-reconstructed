#include "game/world/block/QuartzBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const std::wstring gQuartzTopTexture = L"top";
static const std::wstring gQuartzChiseledTopTexture = L"chiseled_top";
static const std::wstring gQuartzLinesTopTexture = L"lines_top";
static const std::wstring gQuartzBottomTexture = L"bottom";
static const std::wstring gQuartzVariantTextures[5] = {
    L"side",
    L"chiseled",
    L"lines",
    L"",
    L""
};

QuartzBlock::Variant* QuartzBlock::Variant::DEFAULT =
    new QuartzBlock::Variant(0, L"default", L"default");
QuartzBlock::Variant* QuartzBlock::Variant::CHISELED =
    new QuartzBlock::Variant(1, L"chiseled", L"chiseled");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_Y =
    new QuartzBlock::Variant(2, L"lines_y", L"linesy");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_X =
    new QuartzBlock::Variant(3, L"lines_x", L"linesx");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_Z =
    new QuartzBlock::Variant(4, L"lines_z", L"linesz");

static QuartzBlock::Variant* gQuartzVariants[5] = {
    QuartzBlock::Variant::DEFAULT,
    QuartzBlock::Variant::CHISELED,
    QuartzBlock::Variant::LINES_Y,
    QuartzBlock::Variant::LINES_X,
    QuartzBlock::Variant::LINES_Z
};

EnumProperty<QuartzBlock::Variant*>* QuartzBlock::VARIANT = NULL;

QuartzBlock::Variant::Variant(int data, const std::wstring& serializedName, const std::wstring& name)
    : mData(data), mSerializedName(serializedName), mName(name)
{
}

int QuartzBlock::Variant::getData()
{
    return mData;
}

QuartzBlock::Variant* QuartzBlock::Variant::byData(int data)
{
    if (data < 0 || data > 4)
    {
        data = 0;
    }

    return gQuartzVariants[data];
}

std::wstring QuartzBlock::Variant::toString() const
{
    return mName;
}

std::wstring QuartzBlock::Variant::getSerializedName() const
{
    return mSerializedName;
}

void QuartzBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(gQuartzVariants[0]);
    values.insert(gQuartzVariants[1]);
    values.insert(gQuartzVariants[2]);
    values.insert(gQuartzVariants[3]);
    values.insert(gQuartzVariants[4]);

    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant*), values,
                                             Predicates<Variant*>::alwaysTrue);
}

QuartzBlock::QuartzBlock()
    : Block(Material::STONE)
{
    DerivedInit();
    TypedBoxed<Variant*> variant(Variant::DEFAULT);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &variant));
}

const BlockState* QuartzBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction,
                                                    float, float, float, int auxValue,
                                                    boost::shared_ptr<LivingEntity>)
{
    if (auxValue == Variant::LINES_Y->getData())
    {
        int axis = direction->getAxis()->getId();
        if (axis == Direction::Axis::eAxis_X)
        {
            TypedBoxed<Variant*> linesX(Variant::LINES_X);
            return defaultBlockState()->setBoxedValue(VARIANT, &linesX);
        }
        if (axis == Direction::Axis::eAxis_Z)
        {
            TypedBoxed<Variant*> linesZ(Variant::LINES_Z);
            return defaultBlockState()->setBoxedValue(VARIANT, &linesZ);
        }

        TypedBoxed<Variant*> linesY(Variant::LINES_Y);
        return defaultBlockState()->setBoxedValue(VARIANT, &linesY);
    }

    if (auxValue == Variant::CHISELED->getData())
    {
        TypedBoxed<Variant*> chiseled(Variant::CHISELED);
        return defaultBlockState()->setBoxedValue(VARIANT, &chiseled);
    }

    TypedBoxed<Variant*> defaultVariant(Variant::DEFAULT);
    return defaultBlockState()->setBoxedValue(VARIANT, &defaultVariant);
}

int QuartzBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant != Variant::LINES_X && variant != Variant::LINES_Z)
    {
        return variant->getData();
    }

    return Variant::LINES_Y->getData();
}

not_null_ptr<ItemInstance> QuartzBlock::getSilkTouchItemInstance(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::LINES_X || variant == Variant::LINES_Z)
    {
        return not_null_ptr<ItemInstance>(
            new ItemInstance(Item::byBlock(this), 1, Variant::LINES_Y->getData()));
    }

    return Block::getSilkTouchItemInstance(state);
}

const MaterialColor* QuartzBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::QUARTZ;
}

const BlockState* QuartzBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &variant);
}

int QuartzBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* QuartzBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() == Rotation::eTYPE_Clockwise90
        || rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        int data = state->getValue(VARIANT)->getData();
        if (data == Variant::LINES_X->getData())
        {
            TypedBoxed<Variant*> linesZ(Variant::LINES_Z);
            return state->setBoxedValue(VARIANT, &linesZ);
        }
        if (data == Variant::LINES_Z->getData())
        {
            TypedBoxed<Variant*> linesX(Variant::LINES_X);
            return state->setBoxedValue(VARIANT, &linesX);
        }
    }

    return state;
}

BlockStateDefinition* QuartzBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* QuartzBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::LINES_Y
        && (direction == Direction::UP || direction == Direction::DOWN))
    {
        return mTextures[6];
    }
    if (variant == Variant::LINES_X
        && (direction == Direction::EAST || direction == Direction::WEST))
    {
        return mTextures[6];
    }
    if (variant == Variant::LINES_Z
        && (direction == Direction::NORTH || direction == Direction::SOUTH))
    {
        return mTextures[6];
    }

    if (variant != Variant::LINES_Y && variant != Variant::LINES_X && variant != Variant::LINES_Z)
    {
        if (direction == Direction::UP)
        {
            return variant == Variant::CHISELED ? mTextures[5] : mTextures[7];
        }
        if (direction == Direction::DOWN)
        {
            return variant == Variant::CHISELED ? mTextures[5] : mTextures[8];
        }
    }

    return mTextures[variant->getData()];
}

void QuartzBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 5; ++i)
    {
        if (gQuartzVariantTextures[i].empty())
        {
            mTextures[i] = mTextures[i - 1];
        }
        else
        {
            mTextures[i] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzVariantTextures[i]);
        }
    }

    mTextures[7] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzTopTexture);
    mTextures[5] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzChiseledTopTexture);
    mTextures[6] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzLinesTopTexture);
    mTextures[8] = iconRegister->registerIcon(getIconName() + L"_" + gQuartzBottomTexture);
}

EBlockRenderShape QuartzBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_QUARTZ;
}
