#include "game/network/ClientboundChatPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/network/FlexiblePacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Level.hpp"

ClientboundChatPacket::ClientboundChatPacket()
{
    message = eChatPacketMessage_Text;
}

ClientboundChatPacket::ClientboundChatPacket(EChatPacketMessage message, int entityType)
{
    this->message = message;

    if (entityType != -1)
    {
        ints.push_back(entityType);
    }
}

ClientboundChatPacket::ClientboundChatPacket(EChatPacketMessage message, int* intParams, unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams)
{
    this->message = message;

    if (intParams != NULL && numIntParams != 0)
    {
        for (unsigned int i = 0; i < numIntParams; ++i)
        {
            ints.push_back(intParams[i]);
        }
    }

    if (stringParams != NULL && numStringParams != 0)
    {
        for (unsigned int i = 0; i < numStringParams; ++i)
        {
            strings.push_back(stringParams[i]);
        }
    }
}

ClientboundChatPacket::ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, int entityType)
{
    this->message = message;

    if (entityType != -1)
    {
        ints.push_back(entityType);
    }

    strings.push_back(text);
}

ClientboundChatPacket::ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, boost::shared_ptr<Entity> entity, const std::wstring& entityName)
{
    int entityType = (int)entity->GetType();

    this->message = message;

    if (entityType != -1)
    {
        ints.push_back(entityType);
    }

    strings.push_back(text);
    strings.push_back(entityName);

    ints.push_back(entity->getId());
}

ClientboundChatPacket::ClientboundChatPacket(const std::wstring& text, EChatPacketMessage message, boost::shared_ptr<Entity> entity, const std::wstring& entityName, const std::wstring& killerName)
{
    int entityType = (int)entity->GetType();

    this->message = message;

    if (entityType != -1)
    {
        ints.push_back(entityType);
    }

    strings.push_back(text);
    strings.push_back(entityName);
    strings.push_back(killerName);

    ints.push_back(entity->getId());
}

ClientboundChatPacket::~ClientboundChatPacket()
{
}

int ClientboundChatPacket::getPacketId()
{
    return 3;
}

void ClientboundChatPacket::read(DataInputStream* input)
{
    message = (EChatPacketMessage)input->readShort();

    FlexiblePacket::read(input);
}

void ClientboundChatPacket::write(DataOutputStream* output)
{
    output->writeShort((short)message);

    FlexiblePacket::write(output);
}

void ClientboundChatPacket::handle(PacketListener* listener)
{
    listener->handleChat(shared_from_this());
}

int ClientboundChatPacket::getEstimatedSize()
{
    return FlexiblePacket::getEstimatedSize() + 4;
}

std::wstring ClientboundChatPacket::getMobCustomName(Level* level)
{
    boost::shared_ptr<Entity> mob = level->getEntity(ints.back());

    if (mob && mob->hasCustomName())
    {
        return mob->getCustomName();
    }

    int entityData = -1;

    if (!ints.empty() && ints.size() > 2)
    {
        entityData = ints[1];
    }

    return g_pApp->getEntityName(static_cast<uint64_t>(static_cast<long long>(ints[0])), entityData);
}

boost::shared_ptr<Packet> ClientboundChatPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundChatPacket());
}
