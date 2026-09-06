#include "game/world/block/PressurePlateBlock.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<bool>* POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));

PressurePlateBlock::PressurePlateBlock(const std::wstring& name, Material* material, Sensitivity sensitivity)
    : BasePressurePlateBlock(name, material), mSensitivity(sensitivity)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<bool> powered(false);
    registerDefaultState(state->setBoxedValue(POWERED, &powered));
}

PressurePlateBlock::~PressurePlateBlock() {}

int PressurePlateBlock::getSignalForState(const BlockState* state)
{
    return state->getValue(POWERED) ? 15 : 0;
}

const BlockState* PressurePlateBlock::setSignalForState(const BlockState* state, int signal)
{
    TypedBoxed<bool> powered(signal > 0);
    return state->setBoxedValue(POWERED, &powered);
}

void PressurePlateBlock::playOnSound(Level* level, const BlockPos& pos)
{
    const SoundEvent* sound = mSoundType == SoundType::WOOD ? SoundEvents::WOOD_PRESSUREPLATE_CLICK_ON : SoundEvents::STONE_PRESSUREPLATE_CLICK_ON;
    level->playSoundToAllPlayers(pos, sound, SoundSource::eSoundSource_Blocks, 0.3f, mSoundType == SoundType::WOOD ? 0.8f : 0.6f, 16.0f);
}

void PressurePlateBlock::playOffSound(Level* level, const BlockPos& pos)
{
    const SoundEvent* sound = mSoundType == SoundType::WOOD ? SoundEvents::WOOD_PRESSUREPLATE_CLICK_OFF : SoundEvents::STONE_PRESSUREPLATE_CLICK_OFF;
    level->playSoundToAllPlayers(pos, sound, SoundSource::eSoundSource_Blocks, 0.3f, mSoundType == SoundType::WOOD ? 0.7f : 0.5f, 16.0f);
}

int PressurePlateBlock::getSignalStrength(Level* level, const BlockPos& pos)
{
    const AABB* box = getSensitiveAABB(pos);
    std::vector<boost::shared_ptr<Entity> > entities;
    if (mSensitivity == EVERYTHING)
        level->getEntities(boost::shared_ptr<Entity>(), box, &entities);
    else
    {
        std::vector<boost::shared_ptr<Entity> >* found = level->getEntitiesOfClass(typeid(LivingEntity), box);
        if (found != NULL)
        {
            entities = *found;
            delete found;
        }
    }
    for (std::vector<boost::shared_ptr<Entity> >::const_iterator it = entities.begin(); it != entities.end(); ++it)
        if (*it && !(*it)->isIgnoringBlockTriggers()) return 15;
    return 0;
}

const BlockState* PressurePlateBlock::getBlockState(int data)
{
    TypedBoxed<bool> powered(data == 1);
    return defaultBlockState()->setBoxedValue(POWERED, &powered);
}

int PressurePlateBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(POWERED) ? 1 : 0;
}

BlockStateDefinition* PressurePlateBlock::createBlockStateDefinition()
{
    const Property* properties[] = { POWERED };
    return new BlockStateDefinition(this, properties, 1);
}
