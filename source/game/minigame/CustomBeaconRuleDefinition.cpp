#include "game/minigame/CustomBeaconRuleDefinition.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/PlayerList.hpp"

class BlockEntity;
class Packet;

extern MinecraftServer* gMinecraftServer;

template<typename T>
std::wstring toString(T value);

template<typename T>
T fromString(const std::wstring& text);

CustomBeaconRuleDefinition::CustomBeaconRuleDefinition()
{
    mLength = 0;
    mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(0);
    mPosition = NULL;
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(88), &mLength);
}

CustomBeaconRuleDefinition::~CustomBeaconRuleDefinition()
{
    delete mPosition;
}

void CustomBeaconRuleDefinition::processCustomBeacon(ServerLevel* level)
{
    BlockPos position(getX(), getY(), getZ());
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::dynamic_pointer_cast<BeaconBlockEntity>(blockEntity);
    if (beacon && mDirection != static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(0))
    {
        beacon->setBeamDirection(mDirection);
        beacon->setBeamLength(mLength);
        gMinecraftServer->getPlayers()->broadcastAll(beacon->getUpdatePacket());
    }
}

void CustomBeaconRuleDefinition::deactivateCustomBeacon(ServerLevel* level)
{
    BlockPos position(getX(), getY(), getZ());
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::dynamic_pointer_cast<BeaconBlockEntity>(blockEntity);
    if (beacon && mDirection != static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(0))
    {
        beacon->setBeamDirection(static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(0));
        beacon->setBeamLength(0);
        gMinecraftServer->getPlayers()->broadcastAll(beacon->getUpdatePacket());
    }
}

void CustomBeaconRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                  unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(38), toString(mPosition->x));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(39), toString(mPosition->y));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(40), toString(mPosition->z));
}

void CustomBeaconRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"spawnX") == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos(0, 0, 0);
        }
        mPosition->setX(fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnY") == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos(0, 0, 0);
        }
        mPosition->setY(fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnZ") == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos(0, 0, 0);
        }
        mPosition->setZ(fromString<int>(value));
        return;
    }
    if (name.compare(L"beam_direction") == 0)
    {
        if (value.compare(L"none") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(0);
        }
        else if (value.compare(L"+y") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(1);
        }
        else if (value.compare(L"-y") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(2);
        }
        else if (value.compare(L"+x") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(3);
        }
        else if (value.compare(L"-x") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(4);
        }
        else if (value.compare(L"+z") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(5);
        }
        else if (value.compare(L"-z") == 0)
        {
            mDirection = static_cast<BeaconBlockEntity::eBEAM_DIRECTION>(6);
        }
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

int CustomBeaconRuleDefinition::getX()
{
    return mPosition->x;
}

int CustomBeaconRuleDefinition::getY()
{
    return mPosition->y;
}

int CustomBeaconRuleDefinition::getZ()
{
    return mPosition->z;
}

AABB* CustomBeaconRuleDefinition::getBoundingVolume()
{
    return AABB::newTemp(getX(), getY(), getZ(), getX() + 1, getY() + 1, getZ() + 1);
}

void CustomBeaconRuleDefinition::moveBoundingVolume(int x, int y, int z)
{
    mPosition->setX(getX() + x);
    mPosition->setY(getY() + y);
    mPosition->setZ(getZ() + z);
}

ConsoleGameRules::EGameRuleType CustomBeaconRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(75);
}
