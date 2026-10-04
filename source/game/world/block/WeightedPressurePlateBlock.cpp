#include "game/world/block/WeightedPressurePlateBlock.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/entity/mob/Entity.hpp"

#include <typeinfo>
#include <vector>

AbstractProperty<int>* WeightedPressurePlateBlock::POWER = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"power", 0, 15));

WeightedPressurePlateBlock::WeightedPressurePlateBlock(const std::wstring& name, Material* material, int maxWeight,
                                                       const MaterialColor* materialColor)
    : BasePressurePlateBlock(name, material, materialColor != NULL ? materialColor : material->getColor())
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    const BlockState* defaultState;
    {
        TypedBoxed<int> power(0);
        defaultState = state->setBoxedValue(POWER, &power);
    }
    registerDefaultState(defaultState);
    mMaxWeight = maxWeight;
}

WeightedPressurePlateBlock::~WeightedPressurePlateBlock()
{
}

int WeightedPressurePlateBlock::getSignalStrength(Level* level, const BlockPos& pos)
{
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(Entity), getSensitiveAABB(pos));
    int count = static_cast<int>(entities->size());
    if (count > mMaxWeight)
        count = mMaxWeight;
    delete entities;

    if (count <= 0)
        return 0;

    return Mth::ceil(static_cast<float>(count) / static_cast<float>(mMaxWeight) * 15.0f);
}

void WeightedPressurePlateBlock::playOnSound(Level* level, const BlockPos& pos)
{
    level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::METAL_PRESSUREPLATE_CLICK_ON,
                     SoundSource::eSoundSource_Blocks, 0.3f, 0.90000004f, 16.0f);
}

void WeightedPressurePlateBlock::playOffSound(Level* level, const BlockPos& pos)
{
    level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::METAL_PRESSUREPLATE_CLICK_OFF,
                     SoundSource::eSoundSource_Blocks, 0.3f, 0.75f, 16.0f);
}

int WeightedPressurePlateBlock::getSignalForState(const BlockState* state)
{
    return state->getValue(POWER);
}

const BlockState* WeightedPressurePlateBlock::setSignalForState(const BlockState* state, int signal)
{
    TypedBoxed<int> power(signal);
    return state->setBoxedValue(POWER, &power);
}

int WeightedPressurePlateBlock::getTickDelay(Level*)
{
    return 10;
}

const BlockState* WeightedPressurePlateBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    TypedBoxed<int> power(data);
    return state->setBoxedValue(POWER, &power);
}

int WeightedPressurePlateBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(POWER);
}

BlockStateDefinition* WeightedPressurePlateBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { POWER };
    return new BlockStateDefinition(this, properties, 1);
}
