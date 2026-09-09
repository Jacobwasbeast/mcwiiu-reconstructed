#include "game/core/Sheep.hpp"

#include <cmath>

#include "game/core/DyeColor.hpp"
#include "game/core/Mth.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/EatBlockGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

const DyeColor* gSheepWhite = NULL;
const DyeColor* gSheepOrange = NULL;
const DyeColor* gSheepMagenta = NULL;
const DyeColor* gSheepLightBlue = NULL;
const DyeColor* gSheepYellow = NULL;
const DyeColor* gSheepPink = NULL;
extern const DyeColor* gSheepGray;
const DyeColor* gSheepBrown = NULL;
const DyeColor* gSheepBlack = NULL;
const SoundEvent* gSheepAmbientSound = NULL;
const SoundEvent* gSheepHurtSound = NULL;
const SoundEvent* gSheepDeathSound = NULL;

static const EntityDataAccessor<unsigned char>* gSheepSheared;
static boost::unordered_map<const DyeColor*, arrayWithLength<const float> > gSheepColors;

Sheep::Sheep(Level* level) : Animal(level), mContainer(NULL), mCraftingContainer(), mEatBlockGoal(NULL)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 1.3f);
    mEatAnimationTick = 0;
    mContainer = new SheepContainer();
    mCraftingContainer.reset(new CraftingContainer(mContainer, 2, 1));
}

Sheep::~Sheep()
{
    delete mContainer;
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
    gSheepWhite = DyeColor::WHITE;
    gSheepOrange = DyeColor::ORANGE;
    gSheepMagenta = DyeColor::MAGENTA;
    gSheepLightBlue = DyeColor::LIGHT_BLUE;
    gSheepYellow = DyeColor::YELLOW;
    gSheepPink = DyeColor::PINK;
    gSheepGray = DyeColor::GRAY;
    gSheepBrown = DyeColor::BROWN;
    gSheepBlack = DyeColor::BLACK;
    gSheepAmbientSound = SoundEvents::SHEEP_AMBIENT;
    gSheepHurtSound = SoundEvents::SHEEP_HURT;
    gSheepDeathSound = SoundEvents::SHEEP_DEATH;
    return 0x2d680002ULL;
}

void Sheep::registerGoals()
{
    mEatBlockGoal = new EatBlockGoal(this);

    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PanicGoal(this, 1.25), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 1.0), true);

    static Item* temptItemsData[1] = { Items::wheat };
    arrayWithLength<Item*> temptItems;
    temptItems.data = temptItemsData;
    temptItems.length = 1;
    mGoalSelector.addGoal(3, new TemptGoal(this, 1.1, temptItems, false, arrayWithLength<int>()),
                          true);

    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.1), true);
    mGoalSelector.addGoal(5, mEatBlockGoal, true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
}

void Sheep::customServerAiStep()
{
    mEatAnimationTick = static_cast<EatBlockGoal*>(mEatBlockGoal)->getEatAnimationTick();
    Animal::customServerAiStep();
}

void Sheep::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        mEatAnimationTick = mEatAnimationTick - 1;
        if (mEatAnimationTick < 0)
        {
            mEatAnimationTick = 0;
        }
    }
    Animal::aiStep();
}

void Sheep::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23000000417232513);
}

void Sheep::defineSynchedData()
{
    Animal::defineSynchedData();
    getEntityData()->createDateItem(gSheepSheared, static_cast<unsigned char>(0));
}

const void* Sheep::getDefaultLootTable()
{
    return &BuiltInLootTables::SHEEP;
}

void Sheep::handleEntityEvent(unsigned char event, int data)
{
    if (event == 10)
    {
        mEatAnimationTick = 40;
        return;
    }

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
    Mob::addDataWalkers(fixer, 0x000000002D680002ULL);
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
    return false;
}
