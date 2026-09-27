#include "game/core/SnowMan.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Snowball.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

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
    boost::shared_ptr<LivingEntity> owner =
        boost::dynamic_pointer_cast<LivingEntity>(shared_from_this());
    boost::shared_ptr<Snowball> snowball(new Snowball(mLevel, owner));
    double dx = target->mX - mX;
    double dz = target->mZ - mZ;
    double targetY = target->mY + target->getEyeHeight() - 1.1;
    double distance = Mth::sqrt(dx * dx + dz * dz);
    snowball->shoot(dx, targetY - snowball->mY + distance * 0.2, dz, 1.6f, 12.0f);
    playSound(SoundEvents::SNOWMAN_SHOOT, 1.0f,
              1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
    mLevel->addEntity(snowball);
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
    DATA_PUMPKIN = SynchedEntityData::defineId<unsigned char>(EntityType::SNOW_MAN, EntityType::GOLEM, EntityDataSerializers::BYTE);
}

void SnowMan::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SNOW_MAN);
}

void SnowMan::registerGoals()
{
    mGoalSelector.addGoal(1, new RangedAttackGoal(this, this, 1.25, 20, 10.0f), true);
    mGoalSelector.addGoal(2, new WaterAvoidingRandomStrollGoal(this, 1.0, 0.001f / 100.0f), true);
    mGoalSelector.addGoal(3, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(4, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new NearestAttackableTargetGoal(this, typeid(Mob), 10,
                                                               true, false, EntitySelector::ENEMY_SELECTOR,
                                                               true, false), true);
}

void SnowMan::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(4.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2);
}

void SnowMan::defineSynchedData()
{
    Mob::defineSynchedData();
    getEntityData()->createDateItem(DATA_PUMPKIN, static_cast<unsigned char>(16));
}

bool SnowMan::hasPumpkin()
{
    return (getEntityData()->get(DATA_PUMPKIN) & 16) != 0;
}

void SnowMan::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putBoolean(L"Pumpkin", hasPumpkin());
}

void SnowMan::setPumpkin(bool pumpkin)
{
    unsigned char flags = getEntityData()->get(DATA_PUMPKIN);
    getEntityData()->set(DATA_PUMPKIN, pumpkin ? static_cast<unsigned char>(flags | 16) : static_cast<unsigned char>(flags & 239), true);
}

void SnowMan::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(L"Pumpkin"))
        setPumpkin(tag->getBoolean(L"Pumpkin"));
}

void SnowMan::aiStep()
{
    Mob::aiStep();
    if (mLevel->mIsClientSide)
        return;

    int x = static_cast<int>(std::floor(mX));
    int y = static_cast<int>(std::floor(mY));
    int z = static_cast<int>(std::floor(mZ));

    if (isInWaterRainOrBubble())
        hurt(DamageSource::DROWN, 1.0f);

    if (mLevel->getBiome(BlockPos(x, 0, z))->getTemperature(BlockPos(x, y, z)) > 1.0f)
        hurt(DamageSource::ON_FIRE, 1.0f);

    if (mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
    {
        for (int i = 0; i < 4; ++i)
        {
            int snowX = static_cast<int>(std::floor(mX + ((i % 2) * 2 - 1) * 0.25));
            int snowY = static_cast<int>(std::floor(mY));
            int snowZ = static_cast<int>(std::floor(mZ + (((i / 2) % 2) * 2 - 1) * 0.25));
            BlockPos snowPos(snowX, snowY, snowZ);
            if (mLevel->getBlockState(snowPos)->getMaterial() == Material::AIR &&
                mLevel->getBiome(snowPos)->getTemperature(snowPos) < 0.8f &&
                Blocks::snow_layer->mayPlace(mLevel, snowPos))
                mLevel->setBlockAndUpdate(snowPos, Blocks::snow_layer->defaultBlockState());
        }
    }
}

ResourceLocation* SnowMan::getDefaultLootTable()
{
    return &BuiltInLootTables::SNOWMAN;
}

double SnowMan::getEyeHeight()
{
    return 1.7f;
}

bool SnowMan::hurt(const DamageSource* source, float damage)
{
    boost::shared_ptr<Entity> entity = source->getEntity();
    if (entity && (entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        if (!player->isAllowedToAttackPlayers())
            return false;
    }
    return LivingEntity::hurt(source, damage);
}

unsigned int SnowMan::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    unsigned int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    if (itemId == ItemId::SHEARS && hasPumpkin())
        return IDS_TOOLTIPS_SHEAR;
    if (itemId == ItemId::LEAD)
    {
        if (!isLeashed())
            return IDS_TOOLTIPS_LEASH;
        if (getLeashHolder().get() == tooltipData.m_pPlayer.get())
            return IDS_TOOLTIPS_UNLEASH;
    }
    return IDS_NONE;
}

unsigned int SnowMan::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_pPlayer->isAllowedToAttackPlayers() ? IDS_TOOLTIPS_HIT : IDS_NONE;
}

bool SnowMan::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (item->getItem() == Items::shears && hasPumpkin() && !mLevel->mIsClientSide)
    {
        setPumpkin(false);
        item->hurtAndBreak(1, player);
    }
    return Mob::mobInteract(player, hand);
}

const SoundEvent* SnowMan::getAmbientSound() { return SoundEvents::SNOWMAN_AMBIENT; }
const SoundEvent* SnowMan::getHurtSound(const DamageSource*) { return SoundEvents::SNOWMAN_HURT; }
const SoundEvent* SnowMan::getDeathSound() { return SoundEvents::SNOWMAN_DEATH; }
unsigned long long SnowMan::GetType() { return EntityType::SNOW_MAN; }
