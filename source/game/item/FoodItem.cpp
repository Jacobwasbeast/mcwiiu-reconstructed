#include "game/item/FoodItem.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

FoodItem::FoodItem(int nutrition, float saturationModifier, bool isMeat)
    : Item(), mNutrition(nutrition), mSaturationModifier(saturationModifier), mIsMeat(isMeat)
{
    _init();
}

FoodItem::FoodItem(int nutrition, bool isMeat)
    : Item(), mNutrition(nutrition), mSaturationModifier(0.6f), mIsMeat(isMeat)
{
    _init();
}

FoodItem::~FoodItem()
{
}

void FoodItem::_init()
{
    mIsFastFood = false;
    mEffectProbability = 0.0f;
    mCanAlwaysEat = false;
    mEatEffect = NULL;
}

not_null_ptr<ItemInstance> FoodItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance,
                                                     Level* level,
                                                     boost::shared_ptr<LivingEntity> entity)
{
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        player->awardStat(gStats->get_itemsUsed(Item::VALIDgetId(itemInstance->getItem())),
                          GenericStats::param_itemsUsed(player, itemInstance));
        if (itemInstance->getItem() == Items::rottenFlesh && player->getFoodData()->getFoodLevel() == 0)
        {
            player->awardStat(GenericStats::ironBelly(), GenericStats::param_ironBelly());
        }
        player->SetLastAteKelp(itemInstance->getItem() == Items::kelp);
    }

    if (!level->mIsClientSide)
    {
        if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            player->getFoodData()->eat(this, itemInstance);
            level->playSound(boost::shared_ptr<Player>(),
                             player->mX,
                             player->mY,
                             player->mZ,
                             SoundEvents::PLAYER_BURP,
                             SoundSource::eSoundSource_Players,
                             0.5f,
                             level->mRandom->nextFloat() * 0.1f + 0.9f,
                             16.0f);
        }

        bool consume = true;
        if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if (player->mAbilities.mInstabuild)
            {
                consume = false;
            }
        }
        if (consume)
        {
            itemInstance->shrink(1);
        }
    }

    return itemInstance;
}

void FoodItem::addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player)
{
    if (!level->mIsClientSide && mEatEffect != NULL && level->mRandom->nextFloat() < mEffectProbability)
    {
        player->addEffect(new MobEffectInstance(mEatEffect), boost::shared_ptr<Entity>());
    }
}

int FoodItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return mIsFastFood ? 16 : 32;
}

int FoodItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return 1;
}

InteractionResultHolder FoodItem::use(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!canEat(player))
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
    }
    player->startUsingItem(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool FoodItem::canEat(boost::shared_ptr<Player> player)
{
    return player->canEat(mCanAlwaysEat);
}

unsigned int FoodItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return canEat(tooltipData.m_pPlayer) ? 0xD26C6DB5u : 0xFFFFFFFFu;
}

int FoodItem::getNutrition(not_null_ptr<ItemInstance> itemInstance)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        return Minecraft::GetInstance()->GetMiniGame()->getNutrition(Item::VALIDgetId(itemInstance->getItem()), mNutrition);
    }
    return mNutrition;
}

float FoodItem::getSaturationModifier(not_null_ptr<ItemInstance> itemInstance)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        return Minecraft::GetInstance()->GetMiniGame()->getSaturationModifier(Item::VALIDgetId(itemInstance->getItem()), mSaturationModifier);
    }
    return mSaturationModifier;
}

bool FoodItem::isMeat()
{
    return mIsMeat;
}

FoodItem* FoodItem::setEatEffect(MobEffectInstance* effect, float probability)
{
    delete mEatEffect;
    mEatEffect = effect;
    mEffectProbability = probability;
    return this;
}

void FoodItem::setCanAlwaysEat()
{
    mCanAlwaysEat = true;
}

void FoodItem::setIsFastFood()
{
    mIsFastFood = true;
}
