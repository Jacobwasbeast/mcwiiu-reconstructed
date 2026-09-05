#include "game/item/HoeItem.hpp"

#include "game/core/AttributeModifier.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"

HoeItem::HoeItem(const Item::Tier* tier)
    : Item(), mAttackDamage(0.0f), mTier(tier)
{
    mMaterialData = 1;
    setMaxDamage(tier->getUses());
    mAttackDamage = 1.0f + tier->getAttackDamageBonus();
}

HoeItem::~HoeItem()
{
}

void HoeItem::setBlock(not_null_ptr<ItemInstance> itemInstance,
                       boost::shared_ptr<Player> player,
                       Level* level,
                       const BlockPos& pos,
                       const BlockState* state)
{
    level->setBlock(pos, state, 11, false);
    if (!level->mIsClientSide)
    {
        LivingEntity* livingPlayer = (LivingEntity*)(void*)player.get();
        boost::shared_ptr<LivingEntity> entity(player, livingPlayer);
        itemInstance->hurtAndBreak(1, entity);
    }
}

InteractionResult::EInteractionResult HoeItem::useOn(boost::shared_ptr<Player> player,
                                                      Level* level,
                                                      const BlockPos& pos,
                                                      InteractionHand::EInteractionHand hand,
                                                      const Direction* direction,
                                                      float hitX,
                                                      float hitY,
                                                      float hitZ,
                                                      bool simulate)
{
    LivingEntity* livingPlayer = (LivingEntity*)(void*)player.get();
    not_null_ptr<ItemInstance> itemInstance = livingPlayer->getItemInHand(hand);
    BlockPos targetPos = pos.relative(direction);
    const BlockState* targetState = level->getBlockState(targetPos);
    bool blockUsed =
        targetState->getBlock()->use(level, targetPos, targetState, player, hand, direction,
                                     hitX, hitY, hitZ, simulate);
    if (!blockUsed)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    Block* block = level->getBlockState(pos)->getBlock();
    Block* replacement = NULL;
    bool canTill = true;
    if (direction != Direction::DOWN)
    {
        if (level->getBlockState(pos.above())->getBlock() != Blocks::AIR)
        {
            canTill = false;
        }
        else if (block == Blocks::grass)
        {
            replacement = Blocks::farmland;
        }
        else if (block == Blocks::dirt)
        {
            const BlockState* dirtState = level->getBlockState(pos);
            int data = block->convertBlockStateToLegacyData(dirtState);
            if (data == 1)
            {
                replacement = Blocks::dirt;
            }
            else
            {
                replacement = Blocks::farmland;
            }
        }
        else
        {
            canTill = false;
        }
    }
    else
    {
        canTill = false;
    }

    if (canTill && !simulate)
    {
        setBlock(itemInstance, player, level, pos, replacement->defaultBlockState());
    }
    return canTill ? InteractionResult::eInteractionResult_Success
                   : InteractionResult::eInteractionResult_Pass;
}

bool HoeItem::hurtEnemy(not_null_ptr<ItemInstance> itemInstance,
                        boost::shared_ptr<LivingEntity> target,
                        boost::shared_ptr<LivingEntity> attacker)
{
    itemInstance->hurtAndBreak(1, attacker);
    return true;
}

bool HoeItem::isHandEquipped()
{
    return true;
}

const Item::Tier* HoeItem::getTier()
{
    return mTier;
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* HoeItem::getDefaultAttributeModifiers(
    const EquipmentSlot* slot)
{
    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers =
        Item::getDefaultAttributeModifiers(slot);
    double attackDamage = 0.0;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        attackDamage = Minecraft::GetInstance()->GetMiniGame()->getBaseDamage(getId(this), 0.0);
    }
    if (slot == EquipmentSlot::MAINHAND)
    {
        (*modifiers)[SharedMonsterAttributes::ATTACK_DAMAGE->getId()] =
            new AttributeModifier(eMODIFIER_ID_BaseAttackDamage, attackDamage, 0);
        (*modifiers)[SharedMonsterAttributes::ATTACK_SPEED->getId()] =
            new AttributeModifier(eMODIFIER_ID_BaseAttackSpeed, mAttackDamage - 4.0, 0);
    }
    return modifiers;
}
