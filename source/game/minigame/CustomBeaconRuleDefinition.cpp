#include "game/minigame/CustomBeaconRuleDefinition.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/StringConversion.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/PlayerList.hpp"

CustomBeaconRuleDefinition::CustomBeaconRuleDefinition()
{
    mLength = 0;
    mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NONE;
    mPosition = NULL;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_beam_length, &mLength);
}

CustomBeaconRuleDefinition::~CustomBeaconRuleDefinition()
{
    delete mPosition;
}

void CustomBeaconRuleDefinition::processCustomBeacon(ServerLevel* level)
{
    BlockPos position(mPosition->x, mPosition->y, mPosition->z);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::dynamic_pointer_cast<BeaconBlockEntity>(blockEntity);
    if (beacon && mDirection != BeaconBlockEntity::eBEAM_DIRECTION_NONE)
    {
        beacon->setBeamDirection(mDirection);
        beacon->setBeamLength(mLength);
        gMinecraftServer->getPlayers()->broadcastAll(beacon->getUpdatePacket());
    }
}

void CustomBeaconRuleDefinition::deactivateCustomBeacon(ServerLevel* level)
{
    BlockPos position(mPosition->x, mPosition->y, mPosition->z);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::dynamic_pointer_cast<BeaconBlockEntity>(blockEntity);
    if (beacon && mDirection != BeaconBlockEntity::eBEAM_DIRECTION_NONE)
    {
        beacon->setBeamDirection(BeaconBlockEntity::eBEAM_DIRECTION_NONE);
        beacon->setBeamLength(0);
        gMinecraftServer->getPlayers()->broadcastAll(beacon->getUpdatePacket());
    }
}

void CustomBeaconRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                  unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnX, _toString(mPosition->x));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnY, _toString(mPosition->y));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnZ, _toString(mPosition->z));
}

void CustomBeaconRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"spawnX") == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos(0, 0, 0);
        }
        mPosition->setX(_fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnY") == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos(0, 0, 0);
        }
        mPosition->setY(_fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnZ") == 0)
    {
        if (mPosition == NULL)
        {
            mPosition = new MutableBlockPos(0, 0, 0);
        }
        mPosition->setZ(_fromString<int>(value));
        return;
    }
    if (name.compare(L"beam_direction") == 0)
    {
        if (value.compare(L"none") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NONE;
        }
        else if (value.compare(L"+y") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_Y;
        }
        else if (value.compare(L"-y") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_Y;
        }
        else if (value.compare(L"+x") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_X;
        }
        else if (value.compare(L"-x") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_X;
        }
        else if (value.compare(L"+z") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_POSITIVE_Z;
        }
        else if (value.compare(L"-z") == 0)
        {
            mDirection = BeaconBlockEntity::eBEAM_DIRECTION_NEGATIVE_Z;
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
    mPosition->setX(mPosition->x + x);
    mPosition->setY(mPosition->y + y);
    mPosition->setZ(mPosition->z + z);
}

ConsoleGameRules::EGameRuleType CustomBeaconRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CustomBeacon;
}
