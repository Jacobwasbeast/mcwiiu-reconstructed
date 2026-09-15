#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"

#include <string>

#include "game/core/FoodData.hpp"
#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/IGameRuleAttribute.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/AddItemRuleDefinition.hpp"
#include "game/item/Inventory.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/EMiniGameId.hpp"

std::string formatstr(const char* format, ...);

template<typename T>
std::wstring toString(T value);

template<typename T>
T fromString(const std::wstring& text);

UpdatePlayerRuleDefinition::UpdatePlayerRuleDefinition()
{
    mHealth = 0;
    mFoodLevel = 0;
    mSpawnPosition = NULL;
    mDimension = 0;
    mYRot = 0.0f;
    mXRot = 0.0f;

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(42), &mDimension);
    addFloatAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(36), &mYRot)->setOptional(true);
    addFloatAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(37), &mXRot)->setOptional(true);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(10), &mFoodLevel)->setOptional(true);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(11), &mHealth)->setOptional(true);
}

UpdatePlayerRuleDefinition::UpdatePlayerRuleDefinition(boost::shared_ptr<Player> player)
{
    mHealth = 0;
    mFoodLevel = 0;
    mDimension = 0;
    mSpawnPosition = new MutableBlockPos(0, 0, 0);
    mSpawnPosition->setX(static_cast<int>(player->mX));
    mSpawnPosition->setY(static_cast<int>(player->mY));
    mSpawnPosition->setZ(static_cast<int>(player->mZ));
    mYRot = player->mYRot;
    mXRot = player->mXRot;
}

UpdatePlayerRuleDefinition::~UpdatePlayerRuleDefinition()
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        delete *it;
    }
    delete mSpawnPosition;
}

double UpdatePlayerRuleDefinition::SanitisedYPos(double x, double y, double z,
                                                  boost::shared_ptr<Player> player) const
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    double halfHeight = static_cast<double>(player->mBbHeight) * 0.5;
    double testY = y;
    const BlockState* state = level->getBlockState(BlockPos(x, y + halfHeight, z));

    while (state->isSolidRender())
    {
        testY -= 0.1;
        state = level->getBlockState(BlockPos(x, testY + halfHeight, z));
    }

    state = level->getBlockState(BlockPos(x, testY - halfHeight, z));
    while (!state->isSolidRender())
    {
        testY += 0.1;
        state = level->getBlockState(BlockPos(x, testY - halfHeight, z));
    }

    double lower = testY;
    double upper = y;
    if (y < testY)
    {
        lower = y;
        upper = testY;
    }
    if (5.0 - (upper - lower) < 0.0)
    {
        testY = y;
    }
    return testY;
}

void UpdatePlayerRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* UpdatePlayerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    AddItemRuleDefinition* item = NULL;
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(10))
    {
        item = new AddItemRuleDefinition();
        mItems.push_back(item);
    }
    return item;
}

void UpdatePlayerRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                 unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(38), toString(mSpawnPosition->x));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(39), toString(mSpawnPosition->y));
    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(40), toString(mSpawnPosition->z));
}

void UpdatePlayerRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"spawnX") == 0)
    {
        if (mSpawnPosition == NULL)
        {
            mSpawnPosition = new MutableBlockPos(0, 0, 0);
        }
        mSpawnPosition->setX(fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnY") == 0)
    {
        if (mSpawnPosition == NULL)
        {
            mSpawnPosition = new MutableBlockPos(0, 0, 0);
        }
        mSpawnPosition->setY(fromString<int>(value));
        return;
    }
    if (name.compare(L"spawnZ") == 0)
    {
        if (mSpawnPosition == NULL)
        {
            mSpawnPosition = new MutableBlockPos(0, 0, 0);
        }
        mSpawnPosition->setZ(fromString<int>(value));
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

void UpdatePlayerRuleDefinition::postProcessPlayer(boost::shared_ptr<Player> player)
{
    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(11)))
    {
        player->setHealth(static_cast<float>(mHealth));
    }
    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(10)))
    {
        player->getFoodData()->setFoodLevel(mFoodLevel);
    }

    double x = player->mX;
    double y = player->mY;
    double z = player->mZ;
    float yRot = player->mYRot;
    float xRot = player->mXRot;

    if (mSpawnPosition != NULL)
    {
        player->mDimension = mDimension;
        x = static_cast<double>(mSpawnPosition->x) + 0.5;
        z = static_cast<double>(mSpawnPosition->z) + 0.5;

        if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            y = static_cast<double>(mSpawnPosition->y);
        }
        else if (!Minecraft::InMiniGame(eMiniGameId_Lobby, true)
                 && !Minecraft::InMiniGame(eMiniGameId_Battle, true))
        {
            y = SanitisedYPos(x, static_cast<double>(mSpawnPosition->y) + 0.5
                              + static_cast<double>(player->mBbHeight) * 0.5, z, player);
        }
        else
        {
            y = SanitisedYPos(x, static_cast<double>(mSpawnPosition->y), z, player);
        }
    }

    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(36)))
    {
        yRot = mYRot;
    }
    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(37)))
    {
        xRot = mXRot;
    }
    if (mSpawnPosition != NULL
        || attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(36))
        || attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(37)))
    {
        player->absMoveTo(x, y, z, yRot, xRot);
    }

    for (std::vector<AddItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        (*it)->addItemToContainer(player->mInventory, -1);
    }
}

int UpdatePlayerRuleDefinition::getX() const
{
    return mSpawnPosition->x;
}

int UpdatePlayerRuleDefinition::getY() const
{
    return mSpawnPosition->y;
}

int UpdatePlayerRuleDefinition::getZ() const
{
    return mSpawnPosition->z;
}

double UpdatePlayerRuleDefinition::getXRot() const
{
    return static_cast<double>(mXRot);
}

double UpdatePlayerRuleDefinition::getYRot() const
{
    return static_cast<double>(mYRot);
}

void UpdatePlayerRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    xml.append(formatstr(" spawnX=\"%d\"", mSpawnPosition->x));
    xml.append(formatstr(" spawnY=\"%d\"", mSpawnPosition->y));
    xml.append(formatstr(" spawnZ=\"%d\"", mSpawnPosition->z));

    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(36)))
    {
        WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(36), xml);
    }
    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(37)))
    {
        WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(37), xml);
    }
    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(10)))
    {
        WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(10), xml);
    }
    if (attributeHasValue(static_cast<ConsoleGameRules::EGameRuleAttr>(11)))
    {
        WriteAttributeAsXML(static_cast<ConsoleGameRules::EGameRuleAttr>(11), xml);
    }
}

ConsoleGameRules::EGameRuleType UpdatePlayerRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(28);
}
