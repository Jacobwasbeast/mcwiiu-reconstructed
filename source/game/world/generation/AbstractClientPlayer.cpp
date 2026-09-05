#include "game/world/generation/AbstractClientPlayer.hpp"

#include "game/core/Abilities.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

AbstractClientPlayer::AbstractClientPlayer(Level* level, const std::wstring& name)
    : Player(level, name),
      mElytraRotX(15.0f * 0.01745329238474369f),
      mElytraRotY(0.0f),
      mElytraRotZ(-15.0f * 0.01745329238474369f)
{
    mField0x738 = new ItemCooldowns();
}

AbstractClientPlayer::~AbstractClientPlayer()
{
}

void AbstractClientPlayer::tick()
{
    Player::tick();
}

bool AbstractClientPlayer::isPickable()
{
    bool allowPvP = true;
    Minecraft* minecraft = Minecraft::GetInstance();

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        allowPvP = minecraft->GetMiniGame()->AllowPvP();
    }

    return allowPvP && !isSpectator();
}

bool AbstractClientPlayer::isSpectator()
{
    return GetGameType() == GameType::SPECTATOR;
}

bool AbstractClientPlayer::isCreative()
{
    return GetGameType() == GameType::CREATIVE;
}

double AbstractClientPlayer::getFieldOfViewModifier()
{
    float modifier = 1.0f;
    if (mAbilities.mFlying)
    {
        modifier = 1.1f;
    }

    AttributeInstance* movementSpeed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    modifier *= (float)(movementSpeed->getValue() / mAbilities.getWalkingSpeed() + 1.0) * 0.5f;

    if (mAbilities.getWalkingSpeed() == 0.0f)
    {
        modifier = 1.0f;
    }

    if (isUsingItem())
    {
        not_null_ptr<ItemInstance> useItem = getUseItem();
        Item* item = useItem->getItem();

        if (item == Items::bow || item == Items::arrow || item == Items::tippedArrow)
        {
            float draw = (float)getTicksUsingItem() / (float)BowItem::GetMaxBowDuration();
            if (draw > 1.0f)
            {
                draw = 1.0f;
            }
            else
            {
                draw *= draw;
            }

            modifier *= 1.0f - draw * 0.15f;
        }
    }

    return modifier;
}
