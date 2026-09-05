#include "game/item/FireworksItem.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/item/FireworkChargeItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

std::wstring _toString(int value);

FireworksItem::FireworksItem()
    : Item()
{
}

FireworksItem::~FireworksItem()
{
}

InteractionResult::EInteractionResult FireworksItem::useOn(boost::shared_ptr<Player> player,
                                                            Level* level,
                                                            const BlockPos& pos,
                                                            InteractionHand::EInteractionHand hand,
                                                            const Direction* direction,
                                                            float hitX,
                                                            float hitY,
                                                            float hitZ,
                                                            bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (level->mIsClientSide)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        boost::shared_ptr<FireworksRocketEntity> rocket(
            new FireworksRocketEntity(level,
                                      static_cast<double>(pos.x) + hitX,
                                      static_cast<double>(pos.y) + hitY,
                                      static_cast<double>(pos.z) + hitZ,
                                      itemInstance));
        level->addEntity(rocket);
    }
    else
    {
        boost::shared_ptr<FireworksRocketEntity> rocket(
            new FireworksRocketEntity(level, player, itemInstance));
        level->addEntity(rocket);
    }

    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

InteractionResultHolder FireworksItem::use(Level* level,
                                           boost::shared_ptr<Player> player,
                                           InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        if (!level->mIsClientSide)
        {
            boost::shared_ptr<FireworksRocketEntity> rocket(
                new FireworksRocketEntity(level, player, itemInstance));
            level->addEntity(rocket);
            player->getCooldowns()->addCooldown(this, 10);
            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
        }
        else if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureOverrideHandSwing())
        {
            player->swing(hand);
        }
    }
    else
    {
        if (!player->isUsingItem())
        {
            return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
        }

        if (!level->mIsClientSide)
        {
            boost::shared_ptr<FireworksRocketEntity> rocket(
                new FireworksRocketEntity(level, itemInstance, player));
            level->addEntity(rocket);
            player->getCooldowns()->addCooldown(this, 10);
            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
        }
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

void FireworksItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                    boost::shared_ptr<Player> player,
                                    std::vector<HtmlString>* lines,
                                    bool advanced)
{
    if (!itemInstance->hasTag())
    {
        return;
    }

    CompoundTag* fireworks = itemInstance->getTag()->getCompound(std::wstring(L"Fireworks"));
    if (fireworks == NULL)
    {
        return;
    }

    if (fireworks->contains(std::wstring(L"Flight"), 99))
    {
        std::wstring flight(L10N::GetString(0xA774ACD3));
        flight += L" ";
        flight += _toString(static_cast<int>(fireworks->getByte(std::wstring(L"Flight"))));
        lines->push_back(HtmlString(flight, eMinecraftColour_NOTSET, false, false, 0));
    }

    ListTag* explosions = fireworks->getList(std::wstring(L"Explosions"), 10);
    if (explosions == NULL || explosions->size() <= 0)
    {
        return;
    }

    for (int i = 0; i < explosions->size(); ++i)
    {
        std::vector<HtmlString> chargeLines;
        FireworkChargeItem::appendHoverText(explosions->getCompound(i), &chargeLines);
        for (std::vector<HtmlString>::size_type j = 1; j < chargeLines.size(); ++j)
        {
            chargeLines[j].mIndent = true;
        }
        lines->insert(lines->end(), chargeLines.begin(), chargeLines.end());
    }
}

unsigned int FireworksItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return tooltipData.m_pPlayer->isUsingItem() ? 0x568A07BEu : 0xFFFFFFFFu;
}
