#include "game/minigame/CheckpointRuleDefinition.hpp"

#include "game/minigame/CustomBeaconRuleDefinition.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"
#include "game/core/StringFormat.hpp"

static const wchar_t* sTagNames[] =
{
    L"",
    L"MapOptions",
    L"ApplySchematic",
    L"GenerateStructure",
    L"GenerateBox",
    L"PlaceBlock",
    L"PlaceContainer",
    L"PlaceSpawner",
    L"BiomeOverride",
    L"StartFeature",
    L"AddItem",
    L"AddEnchantment",
    L"WeighedTreasureItem",
    L"RandomItemSet",
    L"DistributeItems",
    L"WorldPosition",
    L"LevelRules",
    L"NamedArea",
    L"ActiveChunkArea",
    L"TargetArea",
    L"ScoreRing",
    L"ThermalArea",
    L"PlayerBoundsVolume",
    L"Killbox",
    L"BlockLayer",
    L"UseBlock",
    L"CollectItem",
    L"CompleteAll",
    L"UpdatePlayer",
    L"OnGameStartSpawnPositions",
    L"OnInitialiseWorld",
    L"SpawnPositionSet",
    L"PopulateContainer",
    L"DegradationSequence",
    L"RandomDissolveDegrade",
    L"DirectionalDegrade",
    L"GrantPermissions",
    L"AllowIn",
    L"LayerGeneration",
    L"LayerAsset",
    L"AnyCombinationOf",
    L"CombinationDefinition",
    L"Variations",
    L"BlockDef",
    L"LayerSize",
    L"UniformSize",
    L"RandomizeSize",
    L"LinearBlendSize",
    L"LayerShape",
    L"BasicShape",
    L"StarShape",
    L"PatchyShape",
    L"RingShape",
    L"SpiralShape",
    L"LayerFill",
    L"BasicLayerFill",
    L"CurvedLayerFill",
    L"WarpedLayerFill",
    L"LayerTheme",
    L"NullTheme",
    L"FilterTheme",
    L"ShaftsTheme",
    L"BasicPatchesTheme",
    L"BlockStackTheme",
    L"RainbowTheme",
    L"TerracottaTheme",
    L"FunctionPatchesTheme",
    L"SimplePatchesTheme",
    L"CarpetTrapTheme",
    L"MushroomBlockTheme",
    L"TextureTheme",
    L"SchematicTheme",
    L"BlockCollisionException",
    L"Powerup",
    L"Checkpoint",
    L"CustomBeacon",
    L"ActiveViewArea"
};

CheckpointRuleDefinition::CheckpointRuleDefinition()
    : mBeacons()
{
    mSpawnPoint = NULL;
    mElytraBoost = -1.0;
    mSequenceID = -1;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_id, &mSequenceID);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_speedBoost, &mElytraBoost);
}

CheckpointRuleDefinition::~CheckpointRuleDefinition()
{
    delete mSpawnPoint;
}

uint32_t CheckpointRuleDefinition::getSequenceID()
{
    return mSequenceID;
}

void CheckpointRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    if (mSpawnPoint != NULL)
    {
        children->push_back(mSpawnPoint);
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        children->push_back(*it);
    }
}

void CheckpointRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children,
                                           ConsoleGameRules::EGameRuleType type)
{
    if (mSpawnPoint != NULL && mSpawnPoint->getActionType() == type)
    {
        children->push_back(mSpawnPoint);
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        if ((*it)->getActionType() == type)
        {
            children->push_back(*it);
        }
    }
}

GameRuleDefinition* CheckpointRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_UpdatePlayer)
    {
        mSpawnPoint = new UpdatePlayerRuleDefinition();
        rule = mSpawnPoint;
    }
    else if (type == ConsoleGameRules::eGameRuleType_CustomBeacon)
    {
        CustomBeaconRuleDefinition* beacon = new CustomBeaconRuleDefinition();
        rule = beacon;
        mBeacons.push_back(beacon);
    }

    return rule;
}

void CheckpointRuleDefinition::setPlayerPosition(boost::shared_ptr<ServerPlayer> player)
{
    player->mConnection->teleport(static_cast<double>(mSpawnPoint->getX()),
                                  static_cast<double>(mSpawnPoint->getY()),
                                  static_cast<double>(mSpawnPoint->getZ()),
                                  mSpawnPoint->getYRot(), mSpawnPoint->getXRot(), true, true);
    player->mXOld = static_cast<double>(mSpawnPoint->getX());
    player->mYOld = static_cast<double>(mSpawnPoint->getY());
    player->mZOld = static_cast<double>(mSpawnPoint->getZ());
}

double CheckpointRuleDefinition::getElytraBoost()
{
    return mElytraBoost;
}

UpdatePlayerRuleDefinition* CheckpointRuleDefinition::GetSpawnPoint()
{
    return mSpawnPoint;
}

bool CheckpointRuleDefinition::hasSpawnPoint()
{
    return mSpawnPoint != NULL;
}

void CheckpointRuleDefinition::setSpawnPoint(boost::shared_ptr<Player> player)
{
    delete mSpawnPoint;
    mSpawnPoint = new UpdatePlayerRuleDefinition(player);
}

void CheckpointRuleDefinition::WriteXMLData(std::string& xml)
{
    ConsoleGameRules::EGameRuleType type = getActionType();
    xml += formatstr("<%ls ", sTagNames[type]);
    WriteAttributesAsXML(xml);

    if (mSpawnPoint == NULL)
    {
        xml += "/>\n";
        return;
    }

    xml += ">\n";
    mSpawnPoint->WriteXMLData(xml);
    xml += formatstr("</%ls>\n", sTagNames[type]);
}

void CheckpointRuleDefinition::processBeacons(ServerLevel* level)
{
    if (mBeacons.empty())
    {
        return;
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        (*it)->processCustomBeacon(level);
    }
}

void CheckpointRuleDefinition::deactivateBeacons(ServerLevel* level)
{
    if (mBeacons.empty())
    {
        return;
    }

    for (std::vector<CustomBeaconRuleDefinition*>::iterator it = mBeacons.begin(); it != mBeacons.end(); ++it)
    {
        (*it)->deactivateCustomBeacon(level);
    }
}

void CheckpointRuleDefinition::WriteAttributesAsXML(std::string& xml)
{
    WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_id, xml);
    if (mElytraBoost > -1.0)
    {
        WriteAttributeAsXML(ConsoleGameRules::eGameRuleAttr_speedBoost, xml);
    }
    NamedAreaRuleDefinition::WriteAttributesAsXML(xml);
}

ConsoleGameRules::EGameRuleType CheckpointRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_Checkpoint;
}
