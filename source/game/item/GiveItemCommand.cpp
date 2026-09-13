#include "game/item/GiveItemCommand.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerUID.hpp"

GiveItemCommand::~GiveItemCommand()
{
}

EGameCommand GiveItemCommand::getId()
{
    return eGameCommand_GiveItem;
}

int GiveItemCommand::getPermissionLevel()
{
    return 2;
}

void GiveItemCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    PlayerUID uid = stream.readPlayerUID();

    int itemId = stream.readInt();
    int count = stream.readInt();
    int auxValue = stream.readInt();

    std::wstring playerName = stream.readUTF();

    boost::shared_ptr<ServerPlayer> player = getPlayer(uid);

    if (player && itemId > 0 && Item::byId(itemId) != NULL)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(itemId), count, auxValue));

        bool added = player->mInventory->add(item);

        int intParams[1];
        intParams[0] = itemId;

        if (added)
        {
            player->mLevel->playSound(boost::shared_ptr<Player>(),
                                      player->mX,
                                      player->mY,
                                      player->mZ,
                                      SoundEvents::ITEM_PICKUP,
                                      SoundSource::eSoundSource_Players,
                                      0.2f,
                                      ((player->getRandom()->nextFloat() - player->getRandom()->nextFloat()) * 0.7f + 1.0f) * 2.0f,
                                      16.0f);

            player->mInventoryMenu->broadcastChanges();
        }

        if (added && item->getCount() <= 0)
        {
            item->setCount(1);

            sender->setCommandStats(CommandStats::AFFECTED_ITEMS, count);

            boost::shared_ptr<ItemEntity> entity = player->drop(item, false);

            if (entity)
            {
                entity->makeFakeItem();
            }
        }
        else
        {
            sender->setCommandStats(CommandStats::AFFECTED_ITEMS, count - item->getCount());

            boost::shared_ptr<ItemEntity> entity = player->drop(item, false);

            if (entity)
            {
                entity->setNoPickUpDelay();
                entity->setOwner(player->getName());
            }
        }

        std::wstring stringParams[2];

        stringParams[0] = L"commands.give.success";
        stringParams[1] = player->getName();

        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_Text, intParams, 1, stringParams, 2);
    }
    byteStream.mBuffer.data = NULL;
    byteStream.mBuffer.length = 0;
    byteStream.mCount = 0;
    byteStream.mMark = 0;
    byteStream.mPosition = 0;
}

boost::shared_ptr<GameCommandPacket> GiveItemCommand::preparePacket(boost::shared_ptr<Player> player, int itemId, int count, int data, const std::wstring& playerName)
{
    if (!player)
    {
        return boost::shared_ptr<GameCommandPacket>();
    }

    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writePlayerUID(player->mXuid);
    stream.writeInt(itemId);
    stream.writeInt(count);
    stream.writeInt(data);
    stream.writeUTF(playerName);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_GiveItem, byteStream.toByteArray()));
}
