#include "game/world/generation/ZombieVillager.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<bool>* ZombieVillager::DATA_CONVERTING_ID = NULL;
const EntityDataAccessor<int>* ZombieVillager::DATA_PROFESSION_ID = NULL;

ZombieVillager::ZombieVillager(Level* level)
    : Zombie(level, false), mConversionTime(0), mConversionPlayer()
{
    fjDerivedCtorCalls();
}

ZombieVillager::~ZombieVillager()
{
}

void ZombieVillager::create(Level* level)
{
    new ZombieVillager(level);
}

void ZombieVillager::setProfession(int profession)
{
    mEntityData->set(DATA_PROFESSION_ID, profession, true);
}

void ZombieVillager::staticCtor()
{
    DATA_CONVERTING_ID = SynchedEntityData::defineId<bool>(0x5D550042ULL, 0x5D550040ULL,
                                                            EntityDataSerializers::BOOLEAN);
    DATA_PROFESSION_ID = SynchedEntityData::defineId<int>(0x5D550042ULL, 0x5D550040ULL,
                                                           EntityDataSerializers::INT);
}

void ZombieVillager::defineSynchedData()
{
    Zombie::defineSynchedData();
    mEntityData->createDateItem(DATA_CONVERTING_ID, false);
    mEntityData->createDateItem(DATA_PROFESSION_ID, 0);
}

int ZombieVillager::getProfession()
{
    int profession = mEntityData->get(DATA_PROFESSION_ID) % 6;
    return profession < 0 ? 0 : profession;
}

void ZombieVillager::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5D550042ULL);
}

bool ZombieVillager::isConverting()
{
    return mEntityData->get(DATA_CONVERTING_ID);
}

void ZombieVillager::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(std::wstring(L"Profession"), getProfession());
    tag->putInt(std::wstring(L"ConversionTime"), isConverting() ? mConversionTime : -1);
    if (!mConversionPlayer.empty())
        tag->putString(std::wstring(L"ConversionPlayer"), mConversionPlayer);
}

void ZombieVillager::startConverting(const std::wstring& player, int conversionTime)
{
    int difficulty = mLevel->getDifficulty()->getId() - 1;
    if (difficulty < 0)
        difficulty = 0;
    mConversionPlayer = player;
    mConversionTime = conversionTime;
    mEntityData->set(DATA_CONVERTING_ID, true, true);
    addEffect(new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, conversionTime, difficulty), boost::shared_ptr<Entity>());
}

void ZombieVillager::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setProfession(tag->getInt(std::wstring(L"Profession")));
    if (tag->contains(std::wstring(L"ConversionTime"), 99) &&
        tag->getInt(std::wstring(L"ConversionTime")) >= 0)
    {
        std::wstring player;
        if (tag->contains(std::wstring(L"ConversionPlayer")))
            player = tag->getString(std::wstring(L"ConversionPlayer"));
        startConverting(player, tag->getInt(std::wstring(L"ConversionTime")));
    }
}

MobGroupData* ZombieVillager::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                                const std::wstring& dataString)
{
    setProfession(mLevel->mRandom->nextInt(6));
    return Zombie::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

int ZombieVillager::getConversionProgress()
{
    int progress = 1;
    if (mRandom->nextFloat() < 0.01f)
    {
        int checked = 0;
        for (int x = mX - 4; x < mX + 4 && checked < 14; ++x)
            for (int y = mY - 4; y < mY + 4 && checked < 14; ++y)
                for (int z = mZ - 4; z < mZ + 4 && checked < 14; ++z)
                {
                    const BlockState* state = mLevel->getBlockState(BlockPos(x, y, z));
                    if (state != NULL && (state->getBlock() == Blocks::iron_bars ||
                                           state->getBlock() == Blocks::bed))
                    {
                        if (mRandom->nextFloat() < 0.3f)
                            ++progress;
                        ++checked;
                    }
                }
    }
    return progress;
}

void ZombieVillager::tick()
{
    if (!mLevel->mIsClientSide && isConverting())
    {
        mConversionTime -= getConversionProgress();
        if (mConversionTime < 1)
            finishConversion();
    }
    Zombie::tick();
}

bool ZombieVillager::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    LivingEntity* interactingPlayer = (LivingEntity*)(void*)player.get();
    not_null_ptr<ItemInstance> item = interactingPlayer->getItemInHand(hand);
    if (item->getItem() == Items::goldenApple && item->getAuxValue() == 0 &&
        hasEffect(MobEffects::WEAKNESS) && !mLevel->mIsClientSide && !isConverting())
    {
        item->shrink(1);
        startConverting(interactingPlayer->getStringUUID(), mRandom->nextInt(2400) + 3600);
        return true;
    }
    return Mob::mobInteract(player, hand);
}

unsigned int ZombieVillager::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    return Mob::GetInteractTooltip(holder);
}

bool ZombieVillager::removeWhenFarAway()
{
    return !isConverting();
}

void ZombieVillager::handleEntityEvent(unsigned char event, int data)
{
    if (event == 0x10 && !isConverting())
    {
        mLevel->addParticle(ePARTICLE_HAPPY_VILLAGER, false, mX + 0.5, mY + 0.5,
                            mZ + 0.5, mRandom->nextFloat() + 1.0, mRandom->nextFloat() * 0.7f + 0.3f,
                            0.0, arrayWithLength<int>());
        return;
    }
    Mob::handleEntityEvent(event, data);
}

void ZombieVillager::finishConversion()
{
    boost::shared_ptr<Villager> villager(new Villager(mLevel));
    villager->copyPosition(shared_from_this());
    villager->setArrowCount(getArrowCount());
    villager->setProfession(getProfession());
    villager->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(villager)), NULL, false, std::wstring(L""));
    villager->setRewardPlayersInVillage();
    if (isBaby())
        villager->setAge(-24000);
    villager->setNoAi(isNoAi());
    if (hasCustomName())
    {
        villager->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());
        villager->setCustomNameVisible(isCustomNameVisible());
    }
    mLevel->addEntity(villager);
    boost::shared_ptr<Player> player = mLevel->getPlayerByUUID(mConversionPlayer);
    if (player && player->GetType() == 0x0C800001ULL)
        player->awardStat(GenericStats::zombieDoctor(), GenericStats::param_zombieDoctor());
    villager->addEffect(new MobEffectInstance(MobEffects::CONFUSION, 200, 0), boost::shared_ptr<Entity>());
    mLevel->levelEvent(boost::shared_ptr<Player>(), 0x403, BlockPos(mX, mY, mZ), 0);
}

float ZombieVillager::getVoicePitch()
{
    float variation = mRandom->nextFloat() - mRandom->nextFloat();
    return variation * 0.2f + (isBaby() ? 2.0f : 1.0f);
}

const SoundEvent* ZombieVillager::getAmbientSound()
{
    return SoundEvents::ZOMBIE_VILLAGER_AMBIENT;
}

const SoundEvent* ZombieVillager::getHurtSound(const DamageSource*)
{
    return SoundEvents::ZOMBIE_VILLAGER_HURT;
}

const SoundEvent* ZombieVillager::getDeathSound()
{
    return SoundEvents::ZOMBIE_VILLAGER_DEATH;
}

const SoundEvent* ZombieVillager::getStepSound()
{
    return SoundEvents::ZOMBIE_VILLAGER_STEP;
}

ResourceLocation* ZombieVillager::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE_VILLAGER;
}

ResourceLocation* ZombieVillager::getDefaultGearTable()
{
    return NULL;
}

not_null_ptr<ItemInstance> ZombieVillager::getSkull()
{
    return ItemInstance::EMPTY;
}

unsigned long long ZombieVillager::GetType()
{
    return 0x5D550042ULL;
}
