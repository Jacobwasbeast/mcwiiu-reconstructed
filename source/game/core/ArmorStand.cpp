#include "game/core/ArmorStand.hpp"

#include <cmath>

#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/DataWalker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/Items.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/EntityDataAccessor_Base.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

const EntityDataAccessor<unsigned char>* ArmorStand::DATA_FLAGS = NULL;
const EntityDataAccessor<int>* ArmorStand::DATA_POSE = NULL;

namespace
{
ArmorStand::Pose makeArmorStandPose(const double (&values)[21])
{
    return ArmorStand::Pose(
        Vec3::newPermanent(values[0], values[1], values[2]),
        Vec3::newPermanent(values[3], values[4], values[5]),
        Vec3::newPermanent(values[6], values[7], values[8]),
        Vec3::newPermanent(values[9], values[10], values[11]),
        Vec3::newPermanent(values[12], values[13], values[14]),
        Vec3::newPermanent(values[15], values[16], values[17]),
        Vec3::newPermanent(values[18], values[19], values[20]));
}

const double gArmorStandPoseValues[13][21] = {
    {0,0,0, 0,0,0, -15,0,10, -10,0,-10, 1,0,1, -1,0,-1, 0,0,0},
    {0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0},
    {-5,0,0, 0,0,2, -60,-20,-10, -30,-5,-5, 1,0,1, -1,0,-1, 0,0,0},
    {-5,0,0, 0,0,2, -60,-2,-10, 10,0,-5, 3,3,3, -3,-3,-3, 0,0,0},
    {-15,0,0, 0,0,-2, -110,50,0, 20,0,-10, -5,3,3, 5,-3,-3, 0,0,0},
    {-15,0,0, 0,0,0, -110,-35,0, -30,0,-5, -5,3,3, 5,-3,-3, 0,0,0},
    {-15,0,0, 0,0,0, -110,-35,0, -110,35,0, -5,3,3, 5,-3,-3, 0,0,0},
    {0,0,0, 0,0,0, -70,-40,0, 10,0,-5, 1,0,1, -1,0,-1, 0,0,0},
    {16,20,0, 0,0,0, 246,0,89, 8,0,237, 8,20,4, -14,-18,-16, 0,13,0},
    {-10,-20,0, 0,0,0, -100,0,0, -105,0,0, -46,0,0, 7,0,0, 0,0,0},
    {-5,22,0, 0,22,0, 0,84,111, 8,0,-14, -14,-18,-16, 23,-13,-111, 0,0,0},
    {-10,-20,0, 0,-18,0, 8,-112,111, 0,0,-119, -13,-42,0, 0,0,13, 0,0,0},
    {-4,67,0, 0,8,0, 63,-8,0, 4,0,-99, 67,0,-99, 0,-8,-75, 0,0,0}
};

ArmorStand::Pose gArmorStandPoses[13] = {
    makeArmorStandPose(gArmorStandPoseValues[0]), makeArmorStandPose(gArmorStandPoseValues[1]),
    makeArmorStandPose(gArmorStandPoseValues[2]), makeArmorStandPose(gArmorStandPoseValues[3]),
    makeArmorStandPose(gArmorStandPoseValues[4]), makeArmorStandPose(gArmorStandPoseValues[5]),
    makeArmorStandPose(gArmorStandPoseValues[6]), makeArmorStandPose(gArmorStandPoseValues[7]),
    makeArmorStandPose(gArmorStandPoseValues[8]), makeArmorStandPose(gArmorStandPoseValues[9]),
    makeArmorStandPose(gArmorStandPoseValues[10]), makeArmorStandPose(gArmorStandPoseValues[11]),
    makeArmorStandPose(gArmorStandPoseValues[12])
};
}

ArmorStand::Pose::Pose(const Vec3* headPose, const Vec3* bodyPose,
                       const Vec3* rightArmPose, const Vec3* leftArmPose,
                       const Vec3* rightLegPose, const Vec3* leftLegPose,
                       const Vec3* rightItemPose)
    : head(const_cast<Vec3*>(headPose)), body(const_cast<Vec3*>(bodyPose)),
      leftArm(const_cast<Vec3*>(leftArmPose)), rightArm(const_cast<Vec3*>(rightArmPose)),
      leftLeg(const_cast<Vec3*>(leftLegPose)), rightLeg(const_cast<Vec3*>(rightLegPose)),
      rightItem(const_cast<Vec3*>(rightItemPose))
{
}


ArmorStand::ArmorStand(Level* level) : LivingEntity(level), mHandItems(), mArmorItems()
{
    _init();
}

ArmorStand::ArmorStand(Level* level, double x, double y, double z)
    : LivingEntity(level), mHandItems(), mArmorItems()
{
    _init();
    setPos(x, y, z);
}

ArmorStand::~ArmorStand()
{
}

void ArmorStand::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId<unsigned char>(ENTITY_TYPE_ARMOR_STAND,
                                                             ENTITY_TYPE_LIVING_ENTITY,
                                                             EntityDataSerializers::BYTE);
    DATA_POSE = SynchedEntityData::defineId<int>(ENTITY_TYPE_ARMOR_STAND,
                                                  ENTITY_TYPE_LIVING_ENTITY,
                                                  EntityDataSerializers::INT);
}

void ArmorStand::addDataWalkers(DataFixerUpper* fixer)
{
    std::vector<std::wstring> itemLists;
    itemLists.push_back(L"ArmorItems");
    itemLists.push_back(L"HandItems");
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new ItemListDataWalker(ENTITY_TYPE_ARMOR_STAND, itemLists));
}

void ArmorStand::_init()
{
    fjDerivedCtorCalls();
    mDisabledSlots = 0;
    mInvisible = false;
    mLastHit = 0;
    mWasMarker = false;
    mNoPhysics = isNoGravity();
    setSize(0.5f, 1.975f);
    mHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    mLastSignal = 0;
    setShowArms(true);
}

unsigned char ArmorStand::setBit(unsigned char value, int bit, bool enabled)
{
    return enabled ? (unsigned char)(value | bit) : (unsigned char)(value & ~bit);
}

void ArmorStand::setShowArms(bool showArms)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 4, showArms), true);
}
std::vector<boost::shared_ptr<ItemInstance> >* ArmorStand::getHandSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* items =
        new std::vector<boost::shared_ptr<ItemInstance> >();
    items->push_back(mHandItems[0]);
    items->push_back(mHandItems[1]);
    return items;
}
std::vector<boost::shared_ptr<ItemInstance> >* ArmorStand::getArmorSlots()
{
    std::vector<boost::shared_ptr<ItemInstance> >* items =
        new std::vector<boost::shared_ptr<ItemInstance> >();
    for (unsigned int i = 0; i < mArmorItems.size(); ++i)
        items->push_back(mArmorItems[i]);
    return items;
}
not_null_ptr<ItemInstance> ArmorStand::getItemSlot(const EquipmentSlot* slot)
{
    if (slot->getType() == EquipmentSlot::HAND)
    {
        return mHandItems[slot->getIndex()];
    }
    if (slot->getType() == EquipmentSlot::ARMOR)
    {
        return mArmorItems[slot->getIndex()];
    }
    return ItemInstance::EMPTY;
}
void ArmorStand::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    if (slot->getType() == EquipmentSlot::HAND)
    {
        playEquipSound(item);
        mHandItems[slot->getIndex()] = item;
    }
    else if (slot->getType() == EquipmentSlot::ARMOR)
    {
        playEquipSound(item);
        mArmorItems[slot->getIndex()] = item;
    }
}
bool ArmorStand::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    const EquipmentSlot* equipmentSlot;
    if (slot == 98)
    {
        equipmentSlot = EquipmentSlot::MAINHAND;
    }
    else if (slot == 99)
    {
        equipmentSlot = EquipmentSlot::OFFHAND;
    }
    else if (slot == EquipmentSlot::FEET->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::FEET;
    }
    else if (slot == EquipmentSlot::LEGS->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::LEGS;
    }
    else if (slot == EquipmentSlot::CHEST->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::CHEST;
    }
    else if (slot == EquipmentSlot::HEAD->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::HEAD;
    }
    else
    {
        return false;
    }

    if (!item->isEmpty() && !Mob::isValidSlotForItem(equipmentSlot, item)
        && equipmentSlot != EquipmentSlot::HEAD)
    {
        return false;
    }

    setItemSlot(equipmentSlot, item);
    return true;
}
bool ArmorStand::isEffectiveAi() { return !isMarker() && LivingEntity::isEffectiveAi(); }
bool ArmorStand::isSmall() { return (mEntityData->get(DATA_FLAGS) & 1) != 0; }
bool ArmorStand::isNoBasePlate() { return (mEntityData->get(DATA_FLAGS) & 8) != 0; }
bool ArmorStand::isMarker() { return (mEntityData->get(DATA_FLAGS) & 0x10) != 0; }
int ArmorStand::getPoseIndex() { return mEntityData->get(DATA_POSE); }
void ArmorStand::addAdditonalSaveData(CompoundTag* tag)
{
    ListTag* armorItems = new ListTag();
    unsigned int index;
    for (index = 0; index < mArmorItems.size(); ++index)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mArmorItems[index]->isEmpty())
        {
            mArmorItems[index]->save(itemTag);
        }
        armorItems->add(itemTag);
    }
    tag->put(L"ArmorItems", armorItems);

    ListTag* handItems = new ListTag();
    for (index = 0; index < mHandItems.size(); ++index)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mHandItems[index]->isEmpty())
        {
            mHandItems[index]->save(itemTag);
        }
        handItems->add(itemTag);
    }
    tag->put(L"HandItems", handItems);
    tag->putBoolean(L"Invisible", isInvisible());
    tag->putBoolean(L"Small", isSmall());
    tag->putInt(L"DisabledSlots", mDisabledSlots);
    tag->putBoolean(L"NoGravity", isNoGravity());
    tag->putBoolean(L"NoBasePlate", isNoBasePlate());
    if (isMarker())
    {
        tag->putBoolean(L"Marker", true);
    }

    CompoundTag* pose = new CompoundTag();
    pose->putInt(L"PoseIndex", getPoseIndex());
    pose->putInt(L"LastSignal", mLastSignal);
    tag->put(L"Pose", pose);
}
void ArmorStand::setSmall(bool small)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 1, small), true);
    setSize(0.5f, 1.975f);
}
void ArmorStand::setNoBasePlate(bool noBasePlate)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 8, noBasePlate), true);
}
void ArmorStand::setMarker(bool marker)
{
    mEntityData->set(DATA_FLAGS, setBit(mEntityData->get(DATA_FLAGS), 0x10, marker), true);
    setSize(0.5f, 1.975f);
}
void ArmorStand::setPoseIndex(int pose) { mEntityData->set(DATA_POSE, pose, true); }
void ArmorStand::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"ArmorItems", 9))
    {
        ListTag* armorItems = tag->getList(L"ArmorItems", 10);
        int count = armorItems->size();
        if (count > (int)mArmorItems.size())
        {
            count = (int)mArmorItems.size();
        }
        for (int index = 0; index < count; ++index)
        {
            mArmorItems[index] = not_null_ptr<ItemInstance>(new ItemInstance(armorItems->getCompound(index)));
        }
    }
    if (tag->contains(L"HandItems", 9))
    {
        ListTag* handItems = tag->getList(L"HandItems", 10);
        int count = handItems->size();
        if (count > (int)mHandItems.size())
        {
            count = (int)mHandItems.size();
        }
        for (int index = 0; index < count; ++index)
        {
            mHandItems[index] = not_null_ptr<ItemInstance>(new ItemInstance(handItems->getCompound(index)));
        }
    }

    setInvisible(tag->getBoolean(L"Invisible"));
    setSmall(tag->getBoolean(L"Small"));
    mDisabledSlots = tag->getInt(L"DisabledSlots");
    setNoGravity(tag->getBoolean(L"NoGravity"));
    setNoBasePlate(tag->getBoolean(L"NoBasePlate"));
    setMarker(tag->getBoolean(L"Marker"));
    mWasMarker = !isMarker();
    mNoPhysics = isNoGravity();
    if (tag->contains(L"Pose", 10))
    {
        CompoundTag* pose = tag->getCompound(L"Pose");
        setPoseIndex(pose->getInt(L"PoseIndex"));
        mLastSignal = pose->getInt(L"LastSignal");
    }
}
bool ArmorStand::isPushable(boost::shared_ptr<Entity> entity) { return entity.get() != this; }
const EquipmentSlot* ArmorStand::getClickedSlot(const Vec3* hit)
{
    double y = hit->y;
    bool small = isSmall();
    if (small)
    {
        y *= 2.0;
    }

    if (y >= 0.1 && y < (small ? 0.8 : 0.45) + 0.1 && hasItemInSlot(EquipmentSlot::FEET))
    {
        return EquipmentSlot::FEET;
    }
    if (y >= (small ? 0.3 : 0.05) + 0.9
        && y < (small ? 1.0 : 0.7) + 0.9
        && hasItemInSlot(EquipmentSlot::CHEST))
    {
        return EquipmentSlot::CHEST;
    }
    if (y >= 0.4 && y < (small ? 0.7 : 0.8) + 0.4
        && hasItemInSlot(EquipmentSlot::LEGS))
    {
        return EquipmentSlot::LEGS;
    }
    if (y >= 1.6 && hasItemInSlot(EquipmentSlot::HEAD))
    {
        return EquipmentSlot::HEAD;
    }
    return EquipmentSlot::MAINHAND;
}

bool ArmorStand::isDisabled(const EquipmentSlot* slot)
{
    return (mDisabledSlots & (1 << slot->getFilterFlag())) != 0;
}

bool ArmorStand::swapItem(const boost::shared_ptr<Player>& player, const EquipmentSlot* slot,
                          const not_null_ptr<ItemInstance>& item,
                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
    if (!equipped->isEmpty()
        && (mDisabledSlots & (1 << (slot->getFilterFlag() + 8))) != 0)
    {
        return false;
    }
    if (!equipped->isEmpty()
        && (mDisabledSlots & (1 << (slot->getFilterFlag() + 16))) != 0)
    {
        return false;
    }

    if (!item->isEmpty() && item->getCount() > 1)
    {
        if (!equipped->isEmpty())
        {
            return false;
        }
        not_null_ptr<ItemInstance> singleItem = item->copy();
        singleItem->setCount(1);
        setItemSlot(slot, singleItem);
        item->shrink(1);
        return true;
    }

    setItemSlot(slot, item);
    ((LivingEntity*)player.get())->setItemInHand(hand, equipped);
    return true;
}
bool ArmorStand::isShowArms() { return (mEntityData->get(DATA_FLAGS) & 4) != 0; }
bool ArmorStand::interactAt(const boost::shared_ptr<Player>& player, Vec3* hit,
                            InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> heldItem = ((LivingEntity*)player.get())->getItemInHand(hand);

    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return true;
    }

    if (player->isSneaking())
    {
        setPoseIndex((getPoseIndex() + 1) % 13);
        return false;
    }

    if (isMarker())
    {
        return true;
    }

    if (heldItem->getItem() == Items::nameTag && heldItem->hasCustomHoverName())
    {
        return true;
    }

    if (!mLevel->mIsClientSide && !player->isSpectator())
    {
        const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(heldItem);
        if (!heldItem->isEmpty())
        {
            if (isDisabled(equipmentSlot))
            {
                return true;
            }
            if (equipmentSlot->getType() == EquipmentSlot::HAND && !isShowArms())
            {
                return true;
            }
            swapItem(player, equipmentSlot, heldItem, hand);
        }
        else
        {
            const EquipmentSlot* clickedSlot = getClickedSlot(hit);
            const EquipmentSlot* useSlot = isDisabled(clickedSlot) ? equipmentSlot : clickedSlot;
            if (hasItemInSlot(useSlot))
            {
                swapItem(player, useSlot, heldItem, hand);
            }
        }
    }

    return false;
}
bool ArmorStand::test_interactAt(boost::shared_ptr<Player> player, Vec3* hit,
                                 not_null_ptr<ItemInstance> item,
                                 InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return false;
    }
    if (isMarker())
    {
        return false;
    }
    if (player->isSpectator())
    {
        return false;
    }
    if (player->isSneaking())
    {
        return true;
    }

    return false;
}
unsigned int ArmorStand::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pItemInstance->getItem() == Items::nameTag
        && tooltipData.m_pItemInstance->hasCustomHoverName())
    {
        if (hasCustomName())
        {
            if (tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) == 0)
            {
                if (!tooltipData.m_pPlayer->isSneaking())
                {
                    return 0xffffffffU;
                }
                return 0x3c2d9ad6U;
            }
        }
        return 0x5592c692U;
    }

    return 0xffffffffU;
}
unsigned int ArmorStand::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToHurt ? 0x5fb7a25aU : 0xffffffffU;
}
void ArmorStand::playBrokenSound() { playSound(SoundEvents::ARMORSTAND_BREAK, 1.0f, 1.0f); }
void ArmorStand::brokenByAnything()
{
    playBrokenSound();
    for (std::size_t i = 0; i < mHandItems.size(); ++i)
    {
        if (!mHandItems[i]->isEmpty())
        {
            BlockPos pos(this);
            BlockPos dropPos = pos.above();
            Block::popResource(mLevel, dropPos, mHandItems[i]);
            mHandItems[i] = ItemInstance::EMPTY;
        }
    }
    for (std::size_t i = 0; i < mArmorItems.size(); ++i)
    {
        if (!mArmorItems[i]->isEmpty())
        {
            BlockPos pos(this);
            BlockPos dropPos = pos.above();
            Block::popResource(mLevel, dropPos, mArmorItems[i]);
            mArmorItems[i] = ItemInstance::EMPTY;
        }
    }
}
void ArmorStand::causeDamage(float damage)
{
    float remainingHealth = getHealth() - damage;
    if (remainingHealth <= 0.5f)
    {
        brokenByAnything();
        remove();
    }
    else
    {
        setHealth(remainingHealth);
    }
}
void ArmorStand::showBreakingParticles()
{
    if (mLevel == NULL || mLevel->mIsClientSide)
        return;
    ServerLevel* level = static_cast<ServerLevel*>(mLevel);
    arrayWithLength<int> data(1, false);
    data.data[0] = Block::getIdWithData(Blocks::stone->defaultBlockState());
    level->sendParticles(PARTICLE_BLOCK_DUST, mX, mY + (double)mBbHeight / 2.0, mZ,
                         (int)(mBbHeight * 100.0f), 0.05, 0.05, 0.05, 0.025,
                         data, 0.0, 0.0, 0.0);
}
void ArmorStand::brokenByPlayer()
{
    not_null_ptr<ItemInstance> armorStandItem(new ItemInstance(Items::armorStand));
    BlockPos pos(shared_from_this());
    Block::popResource(mLevel, pos, armorStandItem);
    brokenByAnything();
}
bool ArmorStand::hurt(const DamageSource* source, float damage)
{
    boost::shared_ptr<Entity> sourceEntity = source->getEntity();
    if (sourceEntity && (sourceEntity->GetType() & 0xC800000ULL) == 0xC800000ULL)
    {
        boost::shared_ptr<Player> sourcePlayer = boost::dynamic_pointer_cast<Player>(sourceEntity);
        if (sourcePlayer->isAllowedToHurtEntity(shared_from_this()) == 0)
        {
            return false;
        }
    }

    if (mLevel->mIsClientSide)
    {
        return false;
    }

    if (DamageSource::OUT_OF_WORLD->equals(source))
    {
        remove();
        return false;
    }

    if (isInvulnerableTo(source) || mInvisible || isMarker())
    {
        return false;
    }

    if (source->isExplosion())
    {
        brokenByAnything();
        remove();
        return false;
    }

    if (DamageSource::IN_FIRE->equals(source))
    {
        if (!isOnFire())
        {
            setSecondsOnFire(5);
        }
        else
        {
            causeDamage(0.15f);
        }
        return false;
    }

    if (DamageSource::ON_FIRE->equals(source) && getHealth() > 0.5f)
    {
        causeDamage(4.0f);
        return false;
    }

    const int msgId = source->getMsgId();
    if (msgId != DamageSource::eDamageMsgId_PlayerAttack &&
        msgId != DamageSource::eDamageMsgId_Arrow &&
        msgId != DamageSource::eDamageMsgId_Trident)
    {
        return false;
    }

    boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
    if ((directEntity->GetType() & 0x8A020000ULL) == 0x8A020000ULL)
    {
        directEntity->remove();
    }

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if ((attacker->GetType() & 0xC800000ULL) == 0xC800000ULL)
    {
        boost::shared_ptr<Player> attackingPlayer = boost::dynamic_pointer_cast<Player>(source->getEntity());
        if (!attackingPlayer->mAbilities.mMayBuild)
        {
            return false;
        }
    }

    if (source->isCreativePlayer())
    {
        playBrokenSound();
        showBreakingParticles();
        remove();
        return false;
    }

    long long gameTime = mLevel->getGameTime();
    if (gameTime - mLastHit > 5 &&
        msgId != DamageSource::eDamageMsgId_Arrow &&
        msgId != DamageSource::eDamageMsgId_Trident)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), 32, 0);
        mLastHit = gameTime;
    }
    else
    {
        brokenByPlayer();
        showBreakingParticles();
        remove();
    }
    return false;
}
void ArmorStand::handleEntityEvent(unsigned char event, int data)
{
    if (event == 0x20)
    {
        playBrokenSound();
        mLastHit = mLevel->getGameTime();
    }
    else
    {
        LivingEntity::handleEntityEvent(event, data);
    }
}
bool ArmorStand::shouldRenderAtSqrDistance(double distanceSqr) { return distanceSqr < 4096.0; }
void ArmorStand::setInvisible(bool invisible)
{
    mInvisible = invisible;
    Entity::setInvisible(invisible);
}
bool ArmorStand::isBaby() { return isSmall(); }
void ArmorStand::kill() { remove(); }
bool ArmorStand::ignoreExplosion() { return true; }
int ArmorStand::getPistonPushReaction() { return isMarker() ? 3 : Entity::getPistonPushReaction(); }
bool ArmorStand::isAffectedByPotions() { return false; }
void ArmorStand::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor != NULL && accessor->getId() >= 0)
    {
        setSize(0.5f, 1.975f);
    }
    LivingEntity::onSyncedDataUpdated(accessor);
}
bool ArmorStand::attackable() { return false; }
static ArmorStand::Pose& armorStandPoseForIndex(int index)
{
    if (index < 0) index = 0;
    if (index > 12) index = 12;
    return gArmorStandPoses[index];
}
Vec3* ArmorStand::getHeadPose() { return armorStandPoseForIndex(getPoseIndex()).head; }
Vec3* ArmorStand::getBodyPose() { return armorStandPoseForIndex(getPoseIndex()).body; }
Vec3* ArmorStand::getLeftArmPose() { return armorStandPoseForIndex(getPoseIndex()).leftArm; }
Vec3* ArmorStand::getRightArmPose() { return armorStandPoseForIndex(getPoseIndex()).rightArm; }
Vec3* ArmorStand::getLeftLegPose() { return armorStandPoseForIndex(getPoseIndex()).leftLeg; }
Vec3* ArmorStand::getRightLegPose() { return armorStandPoseForIndex(getPoseIndex()).rightLeg; }
Vec3* ArmorStand::getRightItemPose() { return armorStandPoseForIndex(getPoseIndex()).rightItem; }
bool ArmorStand::isPickable()
{
    return LivingEntity::isPickable() && !isMarker();
}
const HumanoidArm* ArmorStand::getMainArm() { return HumanoidArm::RIGHT; }
float ArmorStand::tickHeadTurn(float bodyRot, float headTurnDelta)
{
    mYBodyRotO = mYRotO;
    mYHeadRotO = mYRotO;
    if (headTurnDelta == 0.0f)
    {
        mYBodyRot = bodyRot;
    }
    return 0.0f;
}
double ArmorStand::getRidingHeight() { return isMarker() ? 0.05 : 1.45; }
double ArmorStand::getEyeHeight() { return (double)(mBbHeight * (isBaby() ? 0.5f : 0.9f)); }
void ArmorStand::travel(float xxa, float yya, float zza)
{
    if (!isNoGravity())
    {
        LivingEntity::travel(xxa, yya, zza);
    }
}
void ArmorStand::setYBodyRot(float bodyRot) { LivingEntity::setYBodyRot(bodyRot); }
void ArmorStand::setYHeadRot(float headRot) { LivingEntity::setYHeadRot(headRot); }
bool ArmorStand::updatePoseForSignal(const BlockPos& pos, const Direction* direction)
{
    BlockPos inputPos = pos.relative(direction->getOpposite());
    if (mLevel->hasSignal(inputPos, direction))
    {
        int signal = mLevel->getSignal(pos.relative(direction), direction);
        if (signal > 0)
        {
            if (signal != mLastSignal)
            {
                int pose = signal > 13 ? 13 : signal;
                if (getPoseIndex() != pose - 1)
                {
                    setPoseIndex(pose - 1);
                }
            }
            mLastSignal = signal;
            return true;
        }
    }
    return false;
}
void ArmorStand::updateBoundingBox(bool marker)
{
    if (marker)
    {
        setSize(0.0f, 0.0f);
    }
    else
    {
        setSize(0.5f, 1.975f);
    }
}
void ArmorStand::tick()
{
    PIXBeginNamedEvent(0.0f, "Armor stand tick");
    if (!mLevel->mIsClientSide)
    {
        BlockPos blockPos(getPos(0.0f));
        arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
        bool poseUpdated = false;
        for (int i = 0; i < directions.length; ++i)
        {
            const Direction* direction = directions.data[i];
            if (!direction->getAxis()->isVertical())
            {
                if (updatePoseForSignal(blockPos.below(), direction))
                {
                    poseUpdated = true;
                    break;
                }
            }
        }
        if (!poseUpdated)
        {
            mLastSignal = 0;
        }
    }
    LivingEntity::tick();
    bool marker = isMarker();
    if (mWasMarker != marker)
    {
        updateBoundingBox(marker);
        mBlocksBuilding = !marker;
    }
    mWasMarker = marker;
    PIXEndNamedEvent();
}
const SoundEvent* ArmorStand::getHurtSound(const DamageSource*) { return SoundEvents::ARMORSTAND_HIT; }
const SoundEvent* ArmorStand::getDeathSound() { return SoundEvents::ARMORSTAND_BREAK; }
const SoundEvent* ArmorStand::getFallDamageSound(int) { return SoundEvents::ARMORSTAND_FALL; }
void ArmorStand::setSize(float width, float height)
{
    double x = mX;
    double y = mY;
    double z = mZ;
    if (isMarker())
    {
        Entity::setSize(width * 0.0f, height * 0.0f);
    }
    else if (isBaby())
    {
        Entity::setSize(width * 0.5f, height * 0.5f);
    }
    else
    {
        Entity::setSize(width, height);
    }
    setPos(x, y, z);
}
void ArmorStand::defineSynchedData()
{
    LivingEntity::defineSynchedData();
    if (DATA_FLAGS != NULL)
        mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
    if (DATA_POSE != NULL)
        mEntityData->createDateItem(DATA_POSE, 0);
}
void ArmorStand::doPush(boost::shared_ptr<Entity>) {}
void ArmorStand::pushEntities()
{
    std::vector<boost::shared_ptr<Entity> >* entities =
        mLevel->getEntities(shared_from_this(), mBoundingBox, NULL, NULL);

    if (entities == NULL || entities->empty())
    {
        return;
    }

    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (distanceToSqr(entity) <= 0.2)
        {
            entity->push(shared_from_this());
        }
    }
}
void ArmorStand::updateInvisibilityStatus() { setInvisible(mInvisible); }
bool ArmorStand::generatesBubbles() { return false; }
void ArmorStand::setAirSupply(int airSupply) { Entity::setAirSupply(airSupply); }
unsigned long long ArmorStand::GetType() { return ENTITY_TYPE_ARMOR_STAND; }
void ArmorStand::thunderHit(const LightningBolt*) {}
void ArmorStand::create(Level* level) { new ArmorStand(level); }

bool ArmorStand::RidableMinecartEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity.get() != NULL && entity->isVehicle();
}

ArmorStand::RidableMinecartEntitySelector::~RidableMinecartEntitySelector()
{
}
