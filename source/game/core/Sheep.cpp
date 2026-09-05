#include "game/core/Sheep.hpp"

#include <cmath>

#include "game/core/DyeColor.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/EatBlockGoal.hpp"

extern const DyeColor* gSheepWhite;
extern const DyeColor* gSheepOrange;
extern const DyeColor* gSheepMagenta;
extern const DyeColor* gSheepLightBlue;
extern const DyeColor* gSheepYellow;
extern const DyeColor* gSheepPink;
extern const DyeColor* gSheepGray;
extern const DyeColor* gSheepBrown;
extern const DyeColor* gSheepBlack;
extern const SoundEvent* gSheepAmbientSound;
extern const SoundEvent* gSheepHurtSound;
extern const SoundEvent* gSheepDeathSound;
extern unsigned int gSheepLootTable;

static const EntityDataAccessor<unsigned char>* gSheepSheared;
static boost::unordered_map<const DyeColor*, arrayWithLength<const float> > gSheepColors;

Sheep::Sheep(Level* level) : Animal(level), mColorData(0), mContainer(), mCraftingContainer(), mEatAnimationTick(0), mEatBlockGoal()
{
    setSize(0.9f, 1.3f);
    registerGoals();
    registerAttributes();
}

Sheep::~Sheep()
{
    mCraftingContainer.reset();
    mContainer.reset();
}

void Sheep::create(Level* level)
{
    new Sheep(level);
}

arrayWithLength<const float> Sheep::getColorArray(const DyeColor* color)
{
    return gSheepColors[color];
}

void Sheep::createSheepColor(const DyeColor* color)
{
    gSheepColors[color] = color->getTextureDiffuseColors();
}

unsigned long long Sheep::staticCtor()
{
    return 0x2d680002ULL;
}

void Sheep::registerGoals()
{
    mEatBlockGoal.reset(new EatBlockGoal(this));
}

void Sheep::customServerAiStep()
{
    if (mEatBlockGoal)
    {
        mEatAnimationTick = static_cast<EatBlockGoal*>(mEatBlockGoal.get())->getEatAnimationTick();
    }
    Animal::customServerAiStep();
}

void Sheep::aiStep()
{
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        if (mEatAnimationTick > 0)
        {
            --mEatAnimationTick;
        }
    }
    Animal::aiStep();
}

void Sheep::registerAttributes()
{
    Mob::registerAttributes();
}

void Sheep::defineSynchedData()
{
    Animal::defineSynchedData();
    if (mEntityData != NULL && gSheepSheared != NULL)
    {
        mEntityData->createDateItem(gSheepSheared, static_cast<unsigned char>(0));
    }
}

unsigned int Sheep::getDefaultLootTable()
{
    return gSheepLootTable;
}

void Sheep::handleEntityEvent(unsigned char event, int data)
{
    Animal::handleEntityEvent(event, data);
}

double Sheep::getHeadEatPositionScale(float partialTick)
{
    if (mEatAnimationTick < 1)
    {
        return 0.0;
    }
    if (mEatAnimationTick < 4)
    {
        return (mEatAnimationTick - partialTick) * 0.25;
    }
    return -((mEatAnimationTick - 40 - partialTick) * 0.25);
}

double Sheep::getHeadEatAngleScale(float partialTick)
{
    if (mEatAnimationTick >= 5 && mEatAnimationTick < 37)
    {
        return static_cast<float>(Mth::sin((mEatAnimationTick - 4 - partialTick) * 0.03125 * 28.7) * 0.2 + 0.4);
    }
    return mEatAnimationTick < 1 ? 0.0 : 0.4;
}

bool Sheep::isSheared()
{
    return mEntityData != NULL && gSheepSheared != NULL && mEntityData->get(gSheepSheared) != 0;
}

void Sheep::setSheared(bool sheared)
{
    if (mEntityData != NULL && gSheepSheared != NULL)
    {
        unsigned char value = mEntityData->get(gSheepSheared);
        mEntityData->set(gSheepSheared, static_cast<unsigned char>((value & 0xf0) | (sheared ? 0x10 : 0)), true);
    }
}

const DyeColor* Sheep::getColor()
{
    return DyeColor::byBlockData(mEntityData != NULL && gSheepSheared != NULL ? mEntityData->get(gSheepSheared) & 0x0f : 0);
}

bool Sheep::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    return Animal::mobInteract(player, hand);
}

void Sheep::addDataWalkers(DataFixerUpper* fixer)
{
    Animal::addDataWalkers(fixer);
}

void Sheep::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Sheared", isSheared());
    tag->putByte(L"Color", static_cast<unsigned char>(getColor()->getBlockData()));
}

void Sheep::setColor(const DyeColor* color)
{
    if (mEntityData != NULL && gSheepSheared != NULL && color != NULL)
    {
        unsigned char value = mEntityData->get(gSheepSheared);
        mEntityData->set(gSheepSheared, static_cast<unsigned char>((value & 0xf0) | color->getBlockData()), true);
    }
}

void Sheep::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    setSheared(tag->getBoolean(L"Sheared"));
    setColor(DyeColor::byBlockData(tag->getByte(L"Color")));
}

unsigned int Sheep::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return Animal::GetInteractTooltip(holder);
}

const SoundEvent* Sheep::getAmbientSound() { return gSheepAmbientSound; }
const SoundEvent* Sheep::getHurtSound(const DamageSource*) { return gSheepHurtSound; }
const SoundEvent* Sheep::getDeathSound() { return gSheepDeathSound; }

void Sheep::playStepSound(const BlockPos& pos, Block* block)
{
    Entity::makeStepSound();
}

unsigned int Sheep::getRandomSheepColor(Random* random)
{
    int value = random->nextInt(100);
    if (value < 5) return gSheepBlack->getBlockData();
    if (value < 10) return gSheepGray->getBlockData();
    if (value < 15) return gSheepLightBlue->getBlockData();
    if (value < 18) return gSheepPink->getBlockData();
    if (random->nextInt(500) == 0) return gSheepBrown->getBlockData();
    return gSheepWhite->getBlockData();
}

const DyeColor* Sheep::getOffspringColor(boost::shared_ptr<Animal> first, boost::shared_ptr<Animal> second)
{
    return DyeColor::byBlockData((first ? static_cast<Sheep*>(first.get())->getColor()->getBlockData() : 0) ^ (second ? static_cast<Sheep*>(second.get())->getColor()->getBlockData() : 0));
}

boost::shared_ptr<AgableMob> Sheep::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<AgableMob> offspring(new Sheep(mLevel));
    boost::shared_ptr<Animal> first = boost::dynamic_pointer_cast<Animal>(shared_from_this());
    boost::shared_ptr<Animal> second = boost::dynamic_pointer_cast<Animal>(partner);
    static_cast<Sheep*>(offspring.get())->setColor(getOffspringColor(first, second));
    return offspring;
}

void Sheep::ate()
{
    setSheared(false);
    if (isBaby())
    {
        ageUp(60);
    }
}

MobGroupData* Sheep::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* group, const std::wstring& name)
{
    MobGroupData* result = Mob::finalizeMobSpawn(difficulty, group, name);
    setColor(DyeColor::byBlockData(getRandomSheepColor(getRandom())));
    return result;
}

double Sheep::getEyeHeight()
{
    return 0.95 * mBaseHeight;
}

unsigned long long Sheep::GetType()
{
    return 0x2d680002ULL;
}

Sheep::SheepContainer::~SheepContainer()
{
}

bool Sheep::SheepContainer::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}
