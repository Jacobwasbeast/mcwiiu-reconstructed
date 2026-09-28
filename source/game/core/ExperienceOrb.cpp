#include "game/core/ExperienceOrb.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

ExperienceOrb::ExperienceOrb(Level* level, double x, double y, double z, int value)
    : Entity(level, true)
{
    _init();
    setSize(0.5f, 0.5f);
    setPos(x, y, z);
    mYRot = (float)(Math::random() * 360.0);
    mXd = (double)((float)(Math::random() * 0.20000000298023224 - 0.10000000149011612) * 2.0f);
    mYd = (double)((float)(Math::random() * 0.2) * 2.0f);
    mZd = (double)((float)(Math::random() * 0.20000000298023224 - 0.10000000149011612) * 2.0f);
    mValue = value;
}

ExperienceOrb::ExperienceOrb(Level* level)
    : Entity(level, true)
{
    _init();
    setSize(0.25f, 0.25f);
}

ExperienceOrb::~ExperienceOrb()
{
}

int ExperienceOrb::getExperienceValue(int value)
{
    if (value > 2476)
    {
        return 2477;
    }
    if (value > 1236)
    {
        return 1237;
    }
    if (value > 616)
    {
        return 617;
    }
    if (value > 306)
    {
        return 307;
    }
    if (value > 148)
    {
        return 149;
    }
    if (value > 72)
    {
        return 73;
    }
    if (value > 36)
    {
        return 37;
    }
    if (value > 16)
    {
        return 17;
    }
    if (value > 6)
    {
        return 7;
    }
    if (value < 3)
    {
        return 1;
    }
    return 3;
}

void ExperienceOrb::_init()
{
    fjDerivedCtorCalls();
    mCount = 0;
    mAge = 0;
    mPickupDelay = 0;
    mHealth = 5;
    mValue = 0;
    mFollowingPlayer.reset();
    mFollowingTime = 0;
}

bool ExperienceOrb::makeStepSound()
{
    return false;
}

void ExperienceOrb::defineSynchedData()
{
}

int ExperienceOrb::getLightColor()
{
    float brightness = Mth::clamp(0.5f, 0.0f, 1.0f);
    int color = Entity::getLightColor();
    int lower = (color & 255) + (int)(brightness * 15.0f * 16.0f);
    if (lower > 240)
    {
        lower = 240;
    }
    return lower | (color & 0x00ff0000);
}

void ExperienceOrb::tick()
{
    Entity::tick();
    if (mPickupDelay > 0)
    {
        --mPickupDelay;
    }

    mXo = mX;
    mYo = mY;
    mZo = mZ;

    if (!isNoGravity())
    {
        mYd -= 0.029999999329447746;
    }

    const BlockState* state = mLevel->getBlockState(BlockPos(shared_from_this()));
    if (state->getMaterial() == Material::LAVA)
    {
        mYd = 0.20000000298023224;
        mXd = (double)((mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f);
        mZd = (double)((mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f);
        playSound(SoundEvents::GENERIC_BURN, 0.4f, mRandom->nextFloat() * 0.4f + 2.0f);
    }

    checkInBlock(mX, (mBoundingBox->mMinY + mBoundingBox->mMaxY) * 0.5, mZ);

    if (mFollowingTime < mCount - 20 + getId() % 100)
    {
        if (!mFollowingPlayer || mFollowingPlayer->distanceToSqr(shared_from_this()) > 64.0)
        {
            mFollowingPlayer = mLevel->getNearestPlayer(shared_from_this(), 8.0, -1.0);
        }
        mFollowingTime = mCount;
    }

    if (mFollowingPlayer && mFollowingPlayer->isSpectator())
    {
        mFollowingPlayer.reset();
    }

    if (mFollowingPlayer)
    {
        double xd = (mFollowingPlayer->mX - mX) / 8.0;
        double yd = (mFollowingPlayer->mY + (double)mFollowingPlayer->getEyeHeight() * 0.5 - mY) / 8.0;
        double zd = (mFollowingPlayer->mZ - mZ) / 8.0;
        double distance = std::sqrt(xd * xd + yd * yd + zd * zd);
        double strength = 1.0 - distance;

        if (strength > 0.0)
        {
            strength *= strength;
            mXd += xd / distance * strength * 0.1;
            mYd += yd / distance * strength * 0.1;
            mZd += zd / distance * strength * 0.1;
        }
    }

    move(eMoverType_Self, mXd, mYd, mZd, false);

    double friction = 0.9800000190734863;
    if (mOnGround)
    {
        int x = Mth::floor(mX);
        int y = Mth::floor(mBoundingBox->mMinY) - 1;
        int z = Mth::floor(mZ);
        friction = (double)mLevel->getBlockState(BlockPos(x, y, z))->getBlock()->mFriction
                 * 0.9800000190734863;
    }

    mXd *= friction;
    mYd *= 0.9800000190734863;
    mZd *= friction;

    if (mOnGround)
    {
        mYd *= -0.8999999761581421;
    }

    ++mCount;
    ++mAge;
    if (mAge > 5999)
    {
        remove();
    }
}

bool ExperienceOrb::updateInWaterState()
{
    return mLevel->checkAndHandleWater(mBoundingBox, Material::WATER, shared_from_this());
}

void ExperienceOrb::burn(int damage)
{
    hurt(DamageSource::IN_FIRE, (float)damage);
}

bool ExperienceOrb::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }
    markHurt();
    if (!mLevel->mIsClientSide)
    {
        mHealth -= (int)damage;
        if (mHealth <= 0)
        {
            remove();
        }
    }
    return false;
}

void ExperienceOrb::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putShort(std::wstring(L"Health"), (short)mHealth);
    tag->putShort(std::wstring(L"Age"), (short)mAge);
    tag->putShort(std::wstring(L"Value"), (short)mValue);
}

void ExperienceOrb::readAdditionalSaveData(CompoundTag* tag)
{
    mHealth = tag->getShort(std::wstring(L"Health")) & 255;
    mAge = tag->getShort(std::wstring(L"Age"));
    mValue = tag->getShort(std::wstring(L"Value"));
}

int ExperienceOrb::xpToDurability(int value)
{
    return value << 1;
}

int ExperienceOrb::durabilityToXp(int value)
{
    return (value >> 1) + ((value < 0 && (value & 1) != 0) ? 1 : 0);
}

void ExperienceOrb::playerTouch(boost::shared_ptr<Player> player)
{
    if (!mLevel->mIsClientSide && mPickupDelay == 0 && player->mTakeXpDelay == 0)
    {
        player->mTakeXpDelay = 2;
        player->take(shared_from_this(), 1);

        boost::shared_ptr<ItemInstance> item = EnchantmentHelper::getRandomItemWith(
            Enchantments::MENDING, boost::static_pointer_cast<LivingEntity>(player));
        if (!item->isEmpty() && item->isDamaged())
        {
            int repaired = xpToDurability(mValue);
            if (repaired > item->getDamageValue())
            {
                repaired = item->getDamageValue();
            }
            mValue -= durabilityToXp(repaired);
            item->setAuxValue(item->getDamageValue() - repaired);
        }

        if (mValue > 0)
        {
            player->increaseXp(mValue);
        }
        remove();
    }
}

int ExperienceOrb::getValue()
{
    return mValue;
}

unsigned char ExperienceOrb::getIcon()
{
    if (mValue > 2476)
    {
        return 10;
    }
    if (mValue > 1236)
    {
        return 9;
    }
    if (mValue > 616)
    {
        return 8;
    }
    if (mValue > 306)
    {
        return 7;
    }
    if (mValue > 148)
    {
        return 6;
    }
    if (mValue > 72)
    {
        return 5;
    }
    if (mValue > 36)
    {
        return 4;
    }
    if (mValue > 16)
    {
        return 3;
    }
    if (mValue > 6)
    {
        return 2;
    }
    return (unsigned char)(mValue > 2);
}

bool ExperienceOrb::isAttackable()
{
    return false;
}

bool ExperienceOrb::shouldRender(double distanceSqr)
{
    return distanceSqr >= 4.0 && Entity::shouldRender(distanceSqr);
}

void ExperienceOrb::create(Level* level)
{
    new ExperienceOrb(level);
}

unsigned long long ExperienceOrb::GetType()
{
    return EntityType::EXPERIENCE_ORB;
}
