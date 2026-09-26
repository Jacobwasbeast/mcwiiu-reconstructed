#include "game/world/LevelRuleset.hpp"

#include <cstdio>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/OnGameStartSpawnPositions.hpp"
#include "game/data/StringTable.hpp"
#include "game/item/RandomItemSetRuleDefinition.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/CollisionExceptionGameRuleDefinition.hpp"
#include "game/minigame/DegradationSequenceRuleDefinition.hpp"
#include "game/minigame/GrantPermissionsRuleDefinition.hpp"
#include "game/minigame/KillboxRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/PowerupRuleDefinition.hpp"
#include "game/minigame/ScoreRingRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/minigame/ActiveViewAreaRuleDefinition.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/world/OnInitialiseWorld.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockLayerRuleDefinition.hpp"
#include "game/world/chunk/ActiveChunkAreaRuleDefinition.hpp"
#include "game/world/generation/LayerGenerationRuleDefinition.hpp"
#include "game/world/generation/PlayerBoundsVolumeRuleDefinition.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/util/AABB.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    int hEvent;
};

template<typename T> T _fromString(const std::wstring& value);
std::string formatstr(const char* format, ...);
char* wstringtofilename(const std::wstring& path, char (&filename)[256]);
int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode,
                void* securityAttributes, unsigned int creationDisposition,
                unsigned int flagsAndAttributes, int templateFile);
int WriteFile(int file, const void* buffer, unsigned int bytesToWrite,
              unsigned int* bytesWritten, _OVERLAPPED* overlapped);
void __debugbreak();

static const int INVALID_HANDLE_VALUE = -1;
static const unsigned int GENERIC_WRITE = 0x40000000;
static const unsigned int CREATE_ALWAYS = 2;
static const unsigned int FILE_FLAG_RANDOM_ACCESS = 0x10000000;

static char s_levelRulesFileName[256];

LevelRuleset::LevelRuleset()
    : CompoundGameRuleDefinition(),
      mGenerationOptions(NULL),
      mRuleType(LevelRules::eLevelRuleType_Unknown),
      mStringTable(NULL)
{
}

LevelRuleset::~LevelRuleset()
{
    std::vector<NamedAreaRuleDefinition*>::iterator namedArea;
    for (namedArea = mNamedAreas.begin(); namedArea != mNamedAreas.end(); ++namedArea)
    {
        delete *namedArea;
    }
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin();
         it != mRandomItemSets.end(); ++it)
    {
        delete *it;
    }
    for (std::vector<CollisionExceptionGameRuleDefinition*>::iterator it = mCollisionExceptions.begin();
         it != mCollisionExceptions.end(); ++it)
    {
        delete *it;
    }
    for (boost::unordered_map<ConsoleGameRules::EGameRuleType, GameRuleDefinition*>::iterator it = mRules.begin();
         it != mRules.end(); ++it)
    {
        delete it->second;
    }
}

RandomItemSetRuleDefinition* LevelRuleset::getRandomItemSet(const std::wstring& name)
{
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end(); ++it)
    {
        if ((*it)->getId() == name)
        {
            return *it;
        }
    }
    return NULL;
}

LevelGenerationOptions* LevelRuleset::getGenerationOptions()
{
    return mGenerationOptions;
}

LevelRules::ELevelRuleType LevelRuleset::getRuleType()
{
    return mRuleType;
}

unsigned int LevelRuleset::getThemeId()
{
    if (mGenerationOptions != NULL)
    {
        return mGenerationOptions->getThemeId();
    }
    return 0xffffffff;
}

void LevelRuleset::getChildren(std::vector<GameRuleDefinition*>* children)
{
    CompoundGameRuleDefinition::getChildren(children);
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        children->push_back(*it);
    }
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end(); ++it)
    {
        children->push_back(*it);
    }
}

void LevelRuleset::addUniqueRule(ConsoleGameRules::EGameRuleType type, GameRuleDefinition* rule)
{
    mRules[type] = rule;
}

GameRuleDefinition* LevelRuleset::addChild(ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case ConsoleGameRules::eGameRuleType_NamedArea:
        {
            NamedAreaRuleDefinition* rule = new NamedAreaRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_ActiveChunkArea:
        {
            ActiveChunkAreaRuleDefinition* rule = new ActiveChunkAreaRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_ActiveViewArea:
        {
            ActiveViewAreaRuleDefinition* rule = new ActiveViewAreaRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_TargetArea:
        {
            TargetAreaRuleDefinition* rule = new TargetAreaRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_ScoreRing:
        {
            ScoreRingRuleDefinition* rule = new ScoreRingRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_ThermalArea:
        {
            ThermalAreaRuleDefinition* rule = new ThermalAreaRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_PlayerBoundsVolume:
        {
            PlayerBoundsVolumeRuleDefinition* rule = new PlayerBoundsVolumeRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_BlockLayer:
        {
            BlockLayerRuleDefinition* rule = new BlockLayerRuleDefinition();
            mNamedAreas.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_RandomItemSet:
        {
            RandomItemSetRuleDefinition* rule = new RandomItemSetRuleDefinition();
            mRandomItemSets.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_OnGameStartSpawnPositions:
        {
            OnGameStartSpawnPositions* rule = new OnGameStartSpawnPositions();
            addUniqueRule(type, rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_OnInitialiseWorld:
        {
            OnInitialiseWorld* rule = new OnInitialiseWorld();
            addUniqueRule(type, rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_DegradationSequence:
        {
            DegradationSequenceRuleDefinition* rule = new DegradationSequenceRuleDefinition();
            addUniqueRule(type, rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_GrantPermissions:
        {
            GrantPermissionsRuleDefinition* rule = new GrantPermissionsRuleDefinition();
            addUniqueRule(type, rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_LayerGeneration:
        {
            LayerGenerationRuleDefinition* rule = new LayerGenerationRuleDefinition();
            addUniqueRule(type, rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_BlockCollisionException:
        {
            CollisionExceptionGameRuleDefinition* rule = new CollisionExceptionGameRuleDefinition();
            mCollisionExceptions.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_Checkpoint:
        {
            CheckpointRuleDefinition* rule = new CheckpointRuleDefinition();
            mCheckpoints.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_Powerup:
        {
            PowerupRuleDefinition* rule = new PowerupRuleDefinition();
            mPowerups.push_back(rule);
            return rule;
        }
    case ConsoleGameRules::eGameRuleType_Killbox:
        {
            KillboxRuleDefinition* rule = new KillboxRuleDefinition(NULL);
            mKillboxes.push_back(rule);
            return rule;
        }
    default:
        return CompoundGameRuleDefinition::addChild(type);
    }
}

void LevelRuleset::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"ruleType")
    {
        mRuleType = static_cast<LevelRules::ELevelRuleType>(_fromString<int>(value));
        return;
    }
    GameRuleDefinition::addAttribute(name, value);
}

void LevelRuleset::loadStringTable(StringTable* stringTable)
{
    mStringTable = stringTable;
}

wchar_t* LevelRuleset::getString(const std::wstring& key)
{
    if (mStringTable == NULL)
    {
        return const_cast<wchar_t*>(L"Active");
    }
    return mStringTable->getString(key);
}

NamedAreaRuleDefinition* LevelRuleset::getNamedAreaRule(const std::wstring& name)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            return *it;
        }
    }
    return NULL;
}

AABB* LevelRuleset::getNamedArea(const std::wstring& name)
{
    NamedAreaRuleDefinition* rule = getNamedAreaRule(name);
    if (rule != NULL)
    {
        return rule->getArea();
    }
    return NULL;
}

void LevelRuleset::getNamedAreas(int areaType, std::vector<AABB*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if (((*it)->mDataTag & areaType) == areaType)
        {
            areas->push_back((*it)->getArea());
        }
    }
}

void LevelRuleset::getNonSpecialisedNamedAreas(std::vector<AABB*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_NamedArea)
        {
            areas->push_back((*it)->getArea());
        }
    }
}

void LevelRuleset::getActiveChunkAreas(std::vector<AABB*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_ActiveChunkArea || (*it)->getName() == L"ActiveArea")
        {
            areas->push_back((*it)->getArea());
        }
    }
}

void LevelRuleset::getActiveChunkAreas(std::vector<NamedAreaRuleDefinition*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_ActiveChunkArea || (*it)->getName() == L"ActiveArea")
        {
            areas->push_back(*it);
        }
    }
}

void LevelRuleset::getActiveViewAreas(std::vector<AABB*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_ActiveViewArea)
        {
            areas->push_back((*it)->getArea());
        }
    }
}

void LevelRuleset::getActiveViewAreas(std::vector<ActiveViewAreaRuleDefinition*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_ActiveViewArea)
        {
            areas->push_back(static_cast<ActiveViewAreaRuleDefinition*>(*it));
        }
    }
}

void LevelRuleset::getTargetAreas(std::vector<TargetAreaRuleDefinition*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        ConsoleGameRules::EGameRuleType type = (*it)->getActionType();
        if (type == ConsoleGameRules::eGameRuleType_TargetArea || type == ConsoleGameRules::eGameRuleType_ScoreRing)
        {
            areas->push_back(static_cast<TargetAreaRuleDefinition*>(*it));
        }
    }
}

void LevelRuleset::getRingsWithinArea(std::vector<TargetAreaRuleDefinition*>* rings, AABB* area)
{
    if (!Minecraft::GetInstance()->GetMiniGame()->ArePointsActive())
    {
        return;
    }

    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        TargetAreaRuleDefinition* rule = static_cast<TargetAreaRuleDefinition*>(*it);
        ConsoleGameRules::EGameRuleType type = rule->getActionType();
        if ((type == ConsoleGameRules::eGameRuleType_TargetArea ||
             type == ConsoleGameRules::eGameRuleType_ScoreRing) && !rule->mPrimary)
        {
            AABB* ringArea = rule->getArea()->grow(1.0, 1.0, 1.0);
            if (area->intersects(ringArea->mMinX, ringArea->mMinY, ringArea->mMinZ,
                                 ringArea->mMaxX, ringArea->mMaxY, ringArea->mMaxZ))
            {
                rings->push_back(rule);
            }
        }
    }
}

void LevelRuleset::getThermalAreas(std::vector<ThermalAreaRuleDefinition*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_ThermalArea)
        {
            areas->push_back(static_cast<ThermalAreaRuleDefinition*>(*it));
        }
    }
}

void LevelRuleset::getPowerups(std::vector<PowerupRuleDefinition*>* powerups)
{
    for (std::vector<PowerupRuleDefinition*>::iterator it = mPowerups.begin(); it != mPowerups.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_Powerup)
        {
            powerups->push_back(*it);
        }
    }
}

void LevelRuleset::getPlayerBoundsVolumes(std::vector<NamedAreaRuleDefinition*>* areas)
{
    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_PlayerBoundsVolume || (*it)->getName() == L"MaxPlayerBounds")
        {
            areas->push_back(*it);
        }
    }
}

void LevelRuleset::getKillboxes(std::vector<KillboxRuleDefinition*>* killboxes)
{
    for (std::vector<KillboxRuleDefinition*>::iterator it = mKillboxes.begin(); it != mKillboxes.end(); ++it)
    {
        if ((*it)->getActionType() == ConsoleGameRules::eGameRuleType_Killbox)
        {
            killboxes->push_back(*it);
        }
    }
}

std::vector<CheckpointRuleDefinition*>* LevelRuleset::getCheckpoints()
{
    return &mCheckpoints;
}

CheckpointRuleDefinition* LevelRuleset::getCheckpointByID(int id)
{
    if (id < static_cast<int>(mCheckpoints.size()))
    {
        return mCheckpoints.at(id);
    }
    return NULL;
}

std::vector<CheckpointRuleDefinition*> LevelRuleset::getCheckpointsBySequenceID(int sequenceId)
{
    std::vector<CheckpointRuleDefinition*> checkpoints;
    for (std::vector<CheckpointRuleDefinition*>::iterator it = mCheckpoints.begin(); it != mCheckpoints.end(); ++it)
    {
        if ((*it)->getSequenceID() == sequenceId)
        {
            checkpoints.push_back(*it);
        }
    }
    return checkpoints;
}

GameRuleDefinition* LevelRuleset::getRuleByType(ConsoleGameRules::EGameRuleType type)
{
    boost::unordered_map<ConsoleGameRules::EGameRuleType, GameRuleDefinition*>::iterator it = mRules.find(type);
    if (it != mRules.end())
    {
        return it->second;
    }
    return NULL;
}
void LevelRuleset::getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type)
{
    CompoundGameRuleDefinition::getChildren(children, type);
    GameRuleDefinition* rule = getRuleByType(type);
    if (rule != NULL)
    {
        children->push_back(rule);
    }
    if ((type > ConsoleGameRules::eGameRuleType_LevelRules &&
         type < ConsoleGameRules::eGameRuleType_BlockLayer) ||
        type == ConsoleGameRules::eGameRuleType_Powerup ||
        type == ConsoleGameRules::eGameRuleType_ActiveViewArea)
    {
        for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
        {
            children->push_back(*it);
        }
    }
    if (type == ConsoleGameRules::eGameRuleType_RandomItemSet)
    {
        for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end(); ++it)
        {
            children->push_back(*it);
        }
    }
}

StringTable* LevelRuleset::getStringTable()
{
    return mStringTable;
}

void LevelRuleset::setGenerationOptions(LevelGenerationOptions* options)
{
    mGenerationOptions = options;
}

bool LevelRuleset::ShouldCollideWith(Block* block)
{
    for (std::vector<CollisionExceptionGameRuleDefinition*>::iterator it = mCollisionExceptions.begin();
         it != mCollisionExceptions.end(); ++it)
    {
        if (!(*it)->ShouldCollideWith(block))
        {
            return false;
        }
    }
    return true;
}

void LevelRuleset::WriteRulesetXMLData()
{
    std::string xml = formatstr("<LevelRules ruleType=\"%d\">\n", static_cast<int>(mRuleType));

    for (std::vector<NamedAreaRuleDefinition*>::iterator it = mNamedAreas.begin(); it != mNamedAreas.end(); ++it)
    {
        (*it)->WriteXMLData(xml);
    }
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin();
         it != mRandomItemSets.end(); ++it)
    {
        (*it)->WriteXMLData(xml);
    }
    for (std::vector<CollisionExceptionGameRuleDefinition*>::iterator it = mCollisionExceptions.begin();
         it != mCollisionExceptions.end(); ++it)
    {
        (*it)->WriteXMLData(xml);
    }
    for (std::vector<CheckpointRuleDefinition*>::iterator it = mCheckpoints.begin(); it != mCheckpoints.end(); ++it)
    {
        (*it)->WriteXMLData(xml);
    }
    for (std::vector<PowerupRuleDefinition*>::iterator it = mPowerups.begin(); it != mPowerups.end(); ++it)
    {
        (*it)->WriteXMLData(xml);
    }
    for (boost::unordered_map<ConsoleGameRules::EGameRuleType, GameRuleDefinition*>::iterator it = mRules.begin();
         it != mRules.end(); ++it)
    {
        it->second->WriteXMLData(xml);
    }

    xml += "</LevelRules>\n";

    std::wstring path = g_pApp->GetDebugRootPath() + L"LevelRules.xml";
    int file = CreateFileA(wstringtofilename(path, s_levelRulesFileName), GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);
    if (file == INVALID_HANDLE_VALUE)
    {
        __debugbreak();
        return;
    }

    unsigned int bytesWritten = 0;
    WriteFile(file, xml.c_str(), static_cast<unsigned int>(xml.length()), &bytesWritten, NULL);
    CloseHandle(file);
}

ConsoleGameRules::EGameRuleType LevelRuleset::getActionType()
{
    return ConsoleGameRules::eGameRuleType_LevelRules;
}
