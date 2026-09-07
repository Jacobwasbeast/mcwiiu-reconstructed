#include "game/item/PowerupItems.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/GameMode.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/ClientboundPowerupPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"

void PowerupItems::DoLargeBoost(boost::shared_ptr<Player> player)
{
    if (player->mLevel->mIsClientSide)
    {
        const double horizontalSpeed = Mth::sqrt(player->mXd * player->mXd + player->mZd * player->mZd);
        player->activateElytraSpeedBoost(horizontalSpeed + 0.6);
    }
}

void PowerupItems::DoSmallBoost(boost::shared_ptr<Player> player)
{
    if (player->mLevel->mIsClientSide)
    {
        const double horizontalSpeed = Mth::sqrt(player->mXd * player->mXd + player->mZd * player->mZd);
        player->activateElytraSpeedBoost(horizontalSpeed + 0.3);
    }
}

void PowerupItems::DoDropTNT(boost::shared_ptr<Player> player)
{
    if (player->mLevel->mIsClientSide)
    {
        return;
    }

    boost::shared_ptr<PrimedTnt> tnt(new PrimedTnt(player->mLevel, player->mX, player->mY,
                                                   player->mZ, player));
    tnt->shouldTick();
    tnt->setFuse(10);
    player->mLevel->addEntity(tnt);
    player->mLevel->explode(tnt, player->mX, player->mY, player->mZ,
                            1.0f, true, false, false, false, false, 16.0f);
}

void PowerupItems::DoGainPoint(boost::shared_ptr<Player> player)
{
    player->GetGameMode()->RecordGainPoints(1);
}

bool PowerupItems::IsDebuff(eGlide_Timed_Powerup_ID powerupId)
{
    return powerupId == eGlide_Timed_Powerup_ID_1;
}

void PowerupItems::BroadcastTimedPowerup(boost::shared_ptr<Player> player,
                                         eGlide_Timed_Powerup_ID powerupId,
                                         int time)
{
    if (!player->mLevel->mIsClientSide)
    {
        PlayerList* players = gMinecraftServer->getPlayers();
        boost::shared_ptr<Packet> packet(new ClientboundPowerupPacket(powerupId, time));
        players->broadcastToAllExceptTeam(player, packet);
    }
}

void PowerupItems::ActivateTimedPowerup(boost::shared_ptr<Player> player,
                                        eGlide_Timed_Powerup_ID powerupId,
                                        int time)
{
    if (player->mLevel->mIsClientSide && player->CheckPowerup(powerupId) == 0)
    {
        if (IsDebuff(powerupId) && player->mAbilities.mInvulnerable)
        {
            return;
        }
        player->SetPowerupTicks(powerupId, time);
    }
}

void PowerupItems::DoTimedPowerup(boost::shared_ptr<Player> player,
                                  eGlide_Timed_Powerup_ID powerupId,
                                  int time)
{
    if (IsDebuff(powerupId))
    {
        BroadcastTimedPowerup(player, powerupId, time);
    }
    else
    {
        ActivateTimedPowerup(player, powerupId, time);
    }
}

void PowerupItems::DoSetInvulnerable(boost::shared_ptr<Player> player, int time)
{
    if (!player->mLevel->mIsClientSide)
    {
        player->mAbilities.mInvulnerable = true;
        PlayerList* players = gMinecraftServer->getPlayers();
        boost::shared_ptr<Packet> packet(new ClientboundPlayerAbilitiesPacket(&player->mAbilities,
                                                                                player->getId()));
        players->broadcastAll(packet);
        player->SetPowerupTicks(eGlide_Timed_Powerup_ID_2, time);
    }
}

void PowerupItems::UsePowerupItem(boost::shared_ptr<Player> player, int itemId)
{
    if (itemId == 0x2e)
    {
        DoDropTNT(player);
    }
    else if (itemId == 0x4c)
    {
        DoSmallBoost(player);
    }
    else if (itemId == 0x120)
    {
        DoTimedPowerup(player, eGlide_Timed_Powerup_ID_0, 0x3c);
    }
    else if (itemId == 0x142)
    {
        DoSetInvulnerable(player, 100);
    }
    else if (itemId == 0x159)
    {
        DoTimedPowerup(player, eGlide_Timed_Powerup_ID_1, 0x3c);
    }
    else if (itemId == 0x179)
    {
        DoLargeBoost(player);
    }
    else if (itemId == 0x184)
    {
        DoGainPoint(player);
    }
}
