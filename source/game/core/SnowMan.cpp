#include "game/core/SnowMan.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/Snowball.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

const EntityDataAccessor<unsigned char>* SnowMan::DATA_PUMPKIN = NULL;

SnowMan::SnowMan(Level* level) : Golem(level)
{
    fjDerivedCtorCalls();
    setSize(0.7f, 1.9f);
}

SnowMan::~SnowMan()
{
}

void SnowMan::create(Level* level)
{
    new SnowMan(level);
}

void SnowMan::performRangedAttack(boost::shared_ptr<LivingEntity> target, float)
{
    if (target == NULL || mLevel == NULL)
        return;
    boost::shared_ptr<Snowball> snowball(new Snowball(mLevel, target));
    double dx = target->mX - mX;
    double dz = target->mZ - mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    double dy = target->mY + target->getEyeHeight() - 1.1 - snowball->mY;
    snowball->shoot(dx, dy + distance * 0.2, dz, 1.6f, 12.0f);
    mLevel->addEntity(boost::static_pointer_cast<Entity>(snowball));
    playSound(SoundEvents::SNOWMAN_SHOOT, 1.0f, 1.0f);
}

bool SnowMan::isAggressive()
{
    return false;
}

void SnowMan::setAggressive(bool)
{
}

void SnowMan::staticCtor()
{
    DATA_PUMPKIN = SynchedEntityData::defineId<unsigned char>(0x2d580001ULL, 0, EntityDataSerializers::BYTE);
}

void SnowMan::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x2d580001ULL);
}

void SnowMan::registerGoals()
{
    mGoalSelector.addGoal(1, new RangedAttackGoal(this, this, 1.25, 20, 10.0f), true);
}

void SnowMan::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(0.7);
}

void SnowMan::defineSynchedData()
{
    Mob::defineSynchedData();
    if (DATA_PUMPKIN != NULL)
        getEntityData()->createDateItem(DATA_PUMPKIN, static_cast<unsigned char>(16));
}

bool SnowMan::hasPumpkin()
{
    return DATA_PUMPKIN != NULL && (getEntityData()->get(DATA_PUMPKIN) & 16) != 0;
}

void SnowMan::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (tag != NULL)
        tag->putBoolean(L"Pumpkin", hasPumpkin());
}

void SnowMan::setPumpkin(bool pumpkin)
{
    if (DATA_PUMPKIN == NULL)
        return;
    unsigned char flags = getEntityData()->get(DATA_PUMPKIN);
    getEntityData()->set(DATA_PUMPKIN, pumpkin ? static_cast<unsigned char>(flags | 16) : static_cast<unsigned char>(flags & 239), true);
}

void SnowMan::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag != NULL && tag->contains(L"Pumpkin"))
        setPumpkin(tag->getBoolean(L"Pumpkin"));
}

void SnowMan::aiStep()
{
    Mob::aiStep();
    if (mLevel == NULL || mLevel->mIsClientSide)
        return;
    BlockPos pos((int)mX, (int)mY, (int)mZ);
    if (isInWaterRainOrBubble())
        setSecondsOnFire(1);
    if (mLevel->getGameRules()->getBoolean(1))
    {
        for (int i = 0; i < 4; ++i)
        {
            int x = (int)mX + (i & 1 ? 1 : -1);
            int z = (int)mZ + (i & 2 ? 1 : -1);
            BlockPos snowPos(x, (int)mY, z);
            if (mLevel->getBlock(snowPos) == Blocks::AIR && Blocks::snow_layer->mayPlace(mLevel, snowPos))
                mLevel->setBlock(snowPos, Blocks::snow_layer->defaultBlockState(), 2, false);
        }
    }
}

const void* SnowMan::getDefaultLootTable()
{
    return &BuiltInLootTables::SNOWMAN;
}

double SnowMan::getEyeHeight()
{
    return 1.7;
}

bool SnowMan::hurt(const DamageSource* source, float damage)
{
    return LivingEntity::hurt(source, damage);
}

unsigned int SnowMan::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    Item* item = tooltipData.m_pItemInstance->getItem();
    if (item == Items::shears && hasPumpkin())
        return 0xbe5aab60U;
    return 0xffffffffU;
}

unsigned int SnowMan::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_pPlayer != NULL ? 0xfdef7118U : 0xffffffffU;
}

bool SnowMan::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    if (item != NULL && item->getItem() == Items::shears && hasPumpkin() && mLevel != NULL && !mLevel->mIsClientSide)
    {
        setPumpkin(false);
        item->hurtAndBreak(1, boost::static_pointer_cast<LivingEntity>(shared_from_this()));
    }
    return Mob::mobInteract(player, hand);
}

const SoundEvent* SnowMan::getAmbientSound() { return SoundEvents::SNOWMAN_AMBIENT; }
const SoundEvent* SnowMan::getHurtSound(const DamageSource*) { return SoundEvents::SNOWMAN_HURT; }
const SoundEvent* SnowMan::getDeathSound() { return SoundEvents::SNOWMAN_DEATH; }
unsigned long long SnowMan::GetType() { return 0x2d58000100000000ULL; }
