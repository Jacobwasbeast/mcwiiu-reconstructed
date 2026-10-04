#include "game/command/CheckpointCommand.hpp"

#include <vector>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerStats.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"

CheckpointCommand::~CheckpointCommand()
{
}

EGameCommand CheckpointCommand::getId()
{
    return eGameCommand_Checkpoint;
}

int CheckpointCommand::getPermissionLevel()
{
    return 0;
}

void CheckpointCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    StatsUID uid(stream.readPlayerUID());

    PlayerList* playerList = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;
    boost::shared_ptr<ServerPlayer> player = playerList->getPlayerFromStatsUID(uid);

    int checkpoint = stream.readInt();

    std::vector<CheckpointRuleDefinition*>* checkpoints =
        gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpoints();

    MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
    unsigned int round = gameMode->mGameStats.mCurrentRound;

    if (player && player->isAlive() && !gameMode->mGameStats.GetPlayerStats(uid)->getReachedEnd(round))
    {
        ServerPlayerGameMode* playerGameMode = static_cast<ServerPlayerGameMode*>(player->GetGameMode());

        if (checkpoint > 0)
        {
            if (!checkpoints->empty() && (unsigned int)checkpoint < checkpoints->size())
            {
                playerGameMode->RecordLastCheckpoint(checkpoint,
                                                     (*checkpoints)[checkpoint]->getSequenceID(), true, true, true);
            }
        }
        else if (checkpoint == 0)
        {
            gameMode->mGameStats.RemovePlayerStats(player);
            gameMode->mGameStats.AddPlayerStats(player);
            playerGameMode->RecordLastCheckpoint(0, 0, true, true, true);
            gameMode->mGameStats.OnRoundStart();

            player->mGameMode->SetDeaths(0);
            player->setHealth(3.0f);
            player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(uid, 0);

            gameMode->ResetRoundTime();
        }

        int lastCheckpoint = playerGameMode->getLastCheckpoint();

        while (lastCheckpoint >= 0 && !(*checkpoints)[lastCheckpoint]->hasSpawnPoint())
        {
            lastCheckpoint--;
        }

        if (lastCheckpoint >= 0)
        {
            playerGameMode->ResetTempPoints();
            playerGameMode->RecordLastCheckpoint(lastCheckpoint,
                                                 (*checkpoints)[lastCheckpoint]->getSequenceID(), true, true, true);

            UpdatePlayerRuleDefinition* spawnPoint = (*checkpoints)[lastCheckpoint]->GetSpawnPoint();
            int spawnX = spawnPoint->getX();
            int spawnY = spawnPoint->getY();
            int spawnZ = spawnPoint->getZ();

            PlayerList* players = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;
            std::vector<boost::shared_ptr<ServerPlayer> >* allPlayers = players->getPlayers();

            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = allPlayers->begin();
                 it != allPlayers->end(); ++it)
            {
                if (*it && (*it)->GetCamera() == player)
                {
                    (*it)->mLoadedChunks.clear();
                    (*it)->mLastSentX = (double)spawnX;
                    (*it)->mLastSentZ = (double)spawnZ;

                    (*it)->mConnection->teleport((double)spawnX, (double)spawnY, (double)spawnZ,
                                                 spawnPoint->getYRot(), spawnPoint->getXRot(), true, true);

                    if (checkpoint == 0)
                    {
                        (*it)->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_UpdateStartTime,
                                                                        GameModePacket::UpdateStartTimeData(0)));
                    }
                }
            }

            player->mXOld = (double)spawnX;
            player->mYOld = (double)spawnY;
            player->mZOld = (double)spawnZ;

            if (CommonMasterGameMode::GetMiniGame()->GetId() == 3)
            {
                player->startFallFlying();
            }
        }

        gameMode->ResetPlayer(player);

        int deaths = gMinecraftServer->mMasterGameMode->mGameStats.GetDeathCount(uid);

        if (gameMode->mGameStats.GetPlayerStats(uid) != NULL)
        {
            player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(uid, deaths);
        }

        player->ResetThermalVariables();
        gameMode->ResetLastKnownPosition(player->mPlayerIndex);
    }
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

boost::shared_ptr<GameCommandPacket> CheckpointCommand::preparePacket(const StatsUID& uid, int checkpoint)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writePlayerUID(uid.GetID());
    stream.writeInt(checkpoint);

    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_Checkpoint, byteStream.toByteArray()));
}
