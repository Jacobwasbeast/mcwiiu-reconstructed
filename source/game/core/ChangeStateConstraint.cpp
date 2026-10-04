#include "game/core/ChangeStateConstraint.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PlayerInfoPacket.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ChangeStateConstraint::ChangeStateConstraint(Tutorial* tutorial, eTutorial_State targetState,
                                             eTutorial_State* states, unsigned int numStates,
                                             double x0, double y0, double z0, double x1, double y1,
                                             double z1, bool insideArea, bool changeGameType,
                                             const GameType* gameType)
    : TutorialConstraint(-1),
      area(AABB::newPermanent(x0, y0, z0, x1, y1, z1)),
      insideArea(insideArea),
      changeGameType(changeGameType),
      gameType(gameType),
      previousGameType(NULL),
      targetState(targetState),
      states(new eTutorial_State[numStates]),
      numStates(numStates),
      stateChanged(false),
      previousState(eTutorial_State_Invalid),
      gameTypeChanged(false),
      tutorial(tutorial)
{
    for (unsigned int i = 0; i < this->numStates; i++)
    {
        this->states[i] = states[i];
    }
}

ChangeStateConstraint::~ChangeStateConstraint()
{
    delete area;

    if (numStates != 0)
    {
        delete[] states;
    }
}

int ChangeStateConstraint::getType()
{
    return 4;
}

void ChangeStateConstraint::tick(int playerId)
{
    Minecraft* minecraft = Minecraft::GetInstance();

    bool inMatchingState = false;
    for (unsigned int i = 0; i < numStates; i++)
    {
        if (states[i] == tutorial->currentState)
        {
            inMatchingState = true;
            break;
        }
    }

    if (stateChanged)
    {
        if (area->contains(minecraft->mLocalPlayers[playerId]->getPos(1.0f)) != insideArea)
        {
            stateChanged = false;
            tutorial->changeTutorialState(previousState, NULL);

            if (changeGameType)
            {
                unsigned int privileges =
                    minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_All);
                Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CreativeMode,
                                               previousGameType == GameType::CREATIVE);

                if (minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_All) != privileges)
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player =
                        minecraft->mLocalPlayers[playerId];

                    if (minecraft->mLocalPlayers[playerId] && minecraft->mLocalPlayers[playerId].get()->mPacketListener != NULL &&
                        minecraft->mLocalPlayers[playerId].get()->mPacketListener->getNetworkPlayer() != NULL)
                    {
                        minecraft->mLocalPlayers[playerId].get()->mPacketListener->send(
                            boost::shared_ptr<Packet>(new PlayerInfoPacket(
                                minecraft->mLocalPlayers[playerId].get()
                                    ->mPacketListener->getNetworkPlayer()
                                    ->GetSmallId(),
                                -1, privileges, player.get()->GetGameType()->getId())));
                    }
                }
            }
        }
    }
    else if (inMatchingState)
    {
        if (area->contains(minecraft->mLocalPlayers[playerId]->getPos(1.0f)) == insideArea)
        {
            previousState = tutorial->currentState;
            stateChanged = true;
            tutorial->changeTutorialState(targetState, NULL);

            if (changeGameType && minecraft->mGameModes[playerId] != NULL)
            {
                if (minecraft->mLocalPlayers[playerId]->mAbilities.mInstabuild)
                {
                    previousGameType = GameType::CREATIVE;
                }
                else
                {
                    previousGameType = GameType::SURVIVAL;
                }

                unsigned int privileges =
                    minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_All);
                Player::setPlayerGamePrivilege(privileges, Player::ePlayerGamePrivilege_CreativeMode,
                                               gameType == GameType::CREATIVE);

                if (minecraft->mLocalPlayers[playerId]->getPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_All) != privileges)
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player =
                        minecraft->mLocalPlayers[playerId];

                    if (minecraft->mLocalPlayers[playerId] && minecraft->mLocalPlayers[playerId].get()->mPacketListener != NULL &&
                        minecraft->mLocalPlayers[playerId].get()->mPacketListener->getNetworkPlayer() != NULL)
                    {
                        minecraft->mLocalPlayers[playerId].get()->mPacketListener->send(
                            boost::shared_ptr<Packet>(new PlayerInfoPacket(
                                minecraft->mLocalPlayers[playerId].get()
                                    ->mPacketListener->getNetworkPlayer()
                                    ->GetSmallId(),
                                -1, privileges, player.get()->GetGameType()->getId())));
                    }
                }
            }
        }
    }
}

bool ChangeStateConstraint::isTargetState(int state)
{
    return state == targetState;
}

bool ChangeStateConstraint::withinTargetStateArea(int playerId)
{
    return area->contains(Minecraft::GetInstance()->mLocalPlayers[playerId]->getPos(1.0f));
}

bool ChangeStateConstraint::withinTargetStateArea(Vec3* pos)
{
    return area->contains(pos);
}
