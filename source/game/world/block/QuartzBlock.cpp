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

QuartzBlock::Variant* QuartzBlock::Variant::DEFAULT =
    new QuartzBlock::Variant(0, L"default", L"default");
QuartzBlock::Variant* QuartzBlock::Variant::CHISELED =
    new QuartzBlock::Variant(1, L"chiseled", L"chiseled");
QuartzBlock::Variant* QuartzBlock::Variant::LINES =
    new QuartzBlock::Variant(2, L"lines", L"lines");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_X =
    new QuartzBlock::Variant(3, L"lines_x", L"lines_x");
QuartzBlock::Variant* QuartzBlock::Variant::LINES_Y =
    new QuartzBlock::Variant(4, L"lines_y", L"lines_y");
EnumProperty<QuartzBlock::Variant*>* QuartzBlock::VARIANT = NULL;
static QuartzBlock::Variant* gQuartzVariants[5] = {
    QuartzBlock::Variant::DEFAULT,
    QuartzBlock::Variant::CHISELED,
    QuartzBlock::Variant::LINES,
    QuartzBlock::Variant::LINES_X,
    QuartzBlock::Variant::LINES_Y
};

QuartzBlock::Variant::Variant(int data, const std::wstring& name, const std::wstring& serializedName)
    : mData(data), mName(name), mSerializedName(serializedName) {}
int QuartzBlock::Variant::getData() { return mData; }
QuartzBlock::Variant* QuartzBlock::Variant::byData(int data)
{
    if (data < 0 || data > 4) data = 0;
    return gQuartzVariants[data];
}
std::wstring QuartzBlock::Variant::toString() const { return mName; }
std::wstring QuartzBlock::Variant::getSerializedName() const { return mSerializedName; }

template std::wstring _toString<QuartzBlock::Variant>(QuartzBlock::Variant*);

void QuartzBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 5; ++i) values.insert(gQuartzVariants[i]);
    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant*), values,
                                               Predicates<Variant*>::alwaysTrue);
}

QuartzBlock::QuartzBlock() : Block(Material::STONE)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::DEFAULT);
    registerDefaultState(defaultBlockState()->setBoxedValue(VARIANT, &value));
    for (int i = 0; i < 9; ++i) mTextures[i] = NULL;
}
QuartzBlock::~QuartzBlock() {}

const BlockState* QuartzBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction, float, float, float, int auxValue, boost::shared_ptr<LivingEntity>)
{
    Variant* variant = Variant::byData(auxValue);
    if (variant == Variant::DEFAULT)
    {
        if (direction->getAxis() == Direction::Axis::Z) variant = Variant::LINES_Y;
        else if (direction->getAxis() == Direction::Axis::X) variant = Variant::LINES_X;
    }
    TypedBoxed<Variant*> value(variant);
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}
int QuartzBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::CHISELED || variant == Variant::LINES_Y) variant = Variant::DEFAULT;
    return variant->getData();
}
not_null_ptr<ItemInstance> QuartzBlock::getSilkTouchItemInstance(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant != Variant::CHISELED && variant != Variant::LINES_Y)
        return Block::getSilkTouchItemInstance(state);
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, Variant::DEFAULT->getData()));
}
const MaterialColor* QuartzBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&) { return MaterialColor::QUARTZ; }
const BlockState* QuartzBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}
int QuartzBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::CHISELED || variant == Variant::LINES_Y) variant = Variant::DEFAULT;
    return variant->getData();
}
const BlockState* QuartzBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() == Rotation::eTYPE_Clockwise90 || rotation->getType() == Rotation::eTYPE_CounterClockwise90)
    {
        Variant* variant = state->getValue(VARIANT);
        if (variant == Variant::LINES_X) { TypedBoxed<Variant*> value(Variant::LINES_Y); return state->setBoxedValue(VARIANT, &value); }
        if (variant == Variant::LINES_Y) { TypedBoxed<Variant*> value(Variant::LINES_X); return state->setBoxedValue(VARIANT, &value); }
    }
    return state;
}
BlockStateDefinition* QuartzBlock::createBlockStateDefinition()
{
    const Property* properties[1] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}
TextureAtlasSprite* QuartzBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == Variant::DEFAULT && (direction == Direction::UP || direction == Direction::DOWN)) return mTextures[7];
    if (variant == Variant::CHISELED && (direction == Direction::NORTH || direction == Direction::SOUTH)) return mTextures[7];
    if (variant == Variant::LINES && (direction == Direction::WEST || direction == Direction::EAST)) return mTextures[7];
    if (variant == Variant::LINES_X && direction == Direction::UP) return mTextures[5];
    if (variant == Variant::LINES_X && direction == Direction::DOWN) return mTextures[8];
    return mTextures[variant->getData()];
}
void QuartzBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    for (int i = 0; i < 5; ++i) mTextures[i] = iconRegister->registerIcon(base + L"_" + gQuartzVariants[i]->getSerializedName());
    mTextures[5] = iconRegister->registerIcon(base + L"_top");
    mTextures[6] = iconRegister->registerIcon(base + L"_chiseled_top");
    mTextures[7] = iconRegister->registerIcon(base + L"_side");
    mTextures[8] = iconRegister->registerIcon(base + L"_lines_top");
}
EBlockRenderShape QuartzBlock::getRenderShape(const BlockState*) { return (EBlockRenderShape)39; }
