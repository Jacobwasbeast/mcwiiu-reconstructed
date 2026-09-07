#include "game/core/GameRuleManager.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/DLCGameRulesHeader.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/GameRulesSource.hpp"
#include "game/data/StringTable.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/LevelGenerators.hpp"

static unsigned int gGameRuleCRCTable[256];
static bool gGameRuleCRCTableReady = false;
extern DLCManager g_DLCManager;

GameRuleManager g_GameRuleManager;

static const wchar_t* gGameRuleTypeNames[77] = {
    L"", L"MapOptions", L"ApplySchematic", L"GenerateStructure", L"GenerateBox", L"PlaceBlock", L"PlaceContainer", L"PlaceSpawner",
    L"BiomeOverride", L"StartFeature", L"AddItem", L"AddEnchantment", L"WeighedTreasureItem", L"RandomItemSet", L"DistributeItems",
    L"WorldPosition", L"LevelRules", L"NamedArea", L"ActiveChunkArea", L"TargetArea", L"ScoreRing", L"ThermalArea", L"PlayerBoundsVolume",
    L"Killbox", L"BlockLayer", L"UseBlock", L"CollectItem", L"CompleteAll", L"UpdatePlayer", L"OnGameStartSpawnPositions",
    L"OnInitialiseWorld", L"SpawnPositionSet", L"PopulateContainer", L"DegradationSequence", L"RandomDissolveDegrade", L"DirectionalDegrade",
    L"GrantPermissions", L"AllowIn", L"LayerGeneration", L"LayerAsset", L"AnyCombinationOf", L"CombinationDefinition", L"Variations", L"BlockDef",
    L"LayerSize", L"UniformSize", L"RandomizeSize", L"LinearBlendSize", L"LayerShape", L"BasicShape", L"StarShape", L"PatchyShape",
    L"RingShape", L"SpiralShape", L"LayerFill", L"BasicLayerFill", L"CurvedLayerFill", L"WarpedLayerFill", L"LayerTheme", L"NullTheme",
    L"FilterTheme", L"ShaftsTheme", L"BasicPatchesTheme", L"BlockStackTheme", L"RainbowTheme", L"TerracottaTheme", L"FunctionPatchesTheme",
    L"SimplePatchesTheme", L"CarpetTrapTheme", L"MushroomBlockTheme", L"TextureTheme", L"SchematicTheme", L"BlockCollisionException",
    L"Powerup", L"Checkpoint", L"CustomBeacon", L"ActiveViewArea"
};

static const wchar_t* gGameRuleAttributeNames[91] = {
    L"descriptionName", L"promptName", L"dataTag", L"enchantmentId", L"enchantmentLevel", L"itemId", L"quantity", L"auxValue", L"slot",
    L"name", L"food", L"health", L"blockId", L"useCoords", L"seed", L"flatworld", L"filename", L"rot", L"data", L"block", L"entity",
    L"facing", L"edgeBlock", L"fillBlock", L"skipAir", L"x", L"x0", L"x1", L"y", L"y0", L"y1", L"z", L"z0", L"z1", L"chunkX",
    L"chunkZ", L"yRot", L"xRot", L"spawnX", L"spawnY", L"spawnZ", L"orientation", L"dimension", L"topblockId", L"biomeId", L"feature",
    L"minCount", L"maxCount", L"weight", L"id", L"probability", L"method", L"hasBeenInCreative", L"cloudHeight", L"fogDistance", L"dayTime",
    L"target", L"speed", L"dir", L"type", L"pass", L"for", L"random", L"blockAux", L"size", L"scale", L"freq", L"func", L"upper", L"lower",
    L"dY", L"thickness", L"points", L"holeSize", L"variant", L"startHeight", L"pattern", L"colour", L"primary", L"laps", L"liftForceModifier",
    L"staticLift", L"targetHeight", L"speedBoost", L"boostDirection", L"condition_type", L"condition_value_0", L"condition_value_1", L"beam_length",
    L"beam_direction", L"string_IDS"
};

GameRuleManager::GameRuleManager()
{
    mLevelGenerationOptions = NULL;
    mDebugGameRuleDefinitions = NULL;
    mGameRuleDefinitions = NULL;
    mLevelGenerators = new LevelGenerators();
    mLevelRules = new LevelRules();
}

void GameRuleManager::unloadCurrentGameRules()
{
    if (mGameRuleDefinitions != NULL)
    {
        if (mLevelGenerationOptions->isFromSave())
        {
            mLevelRules->removeLevelRule(mGameRuleDefinitions);
            delete mGameRuleDefinitions;
        }
    }
    if (mLevelGenerationOptions != NULL)
    {
        if (mLevelGenerationOptions->isFromSave())
        {
            mLevelGenerators->removeLevelGenerator(mLevelGenerationOptions);
            delete mLevelGenerationOptions;
        }
        else if (mLevelGenerationOptions->isFromDLC())
        {
            mLevelGenerationOptions->reset_finish();
        }
    }
    mLevelGenerationOptions = NULL;
    mGameRuleDefinitions = NULL;
}

void GameRuleManager::setLevelGenerationOptions(LevelGenerationOptions* options)
{
    unloadCurrentGameRules();
    mLevelGenerationOptions = options;
    mGameRuleDefinitions = NULL;
    mDebugGameRuleDefinitions = NULL;
    if (options != NULL)
    {
        if (options->requiresGameRules())
        {
            mGameRuleDefinitions = options->getRequiredGameRules();
        }
        options->reset_start();
    }
}

void GameRuleManager::MakeCRCTable()
{
    unsigned int i;
    unsigned int j;
    unsigned int value;
    for (i = 0; i < 256; ++i)
    {
        value = i;
        for (j = 0; j < 8; ++j)
        {
            value = (value & 1) ? ((value >> 1) ^ 0xedb88320U) : (value >> 1);
        }
        gGameRuleCRCTable[i] = value;
    }
    gGameRuleCRCTableReady = true;
}

unsigned int GameRuleManager::UpdateCRC(unsigned int crc, unsigned char* data, int length)
{
    static const unsigned char increments[56] = {
        5,6,7,3,4,5,6,7,1,2,6,1,1,2,3,4,5,5,6,7,3,4,5,6,7,1,2,6,
        1,1,2,3,4,5,5,6,7,3,4,5,6,7,7,5,4,7,5,4,7,5,4,7,6,5,4,3
    };
    int position = 0;
    unsigned int increment = 0;
    if (!gGameRuleCRCTableReady)
    {
        MakeCRCTable();
    }
    while (position < length)
    {
        crc = gGameRuleCRCTable[(crc ^ data[position]) & 0xff] ^ (crc >> 8);
        position += increments[increment];
        increment = (increment + 1) % 56;
    }
    return crc;
}

unsigned int GameRuleManager::CRC(unsigned char* data, int length)
{
    return ~UpdateCRC(0xffffffffU, data, length);
}

unsigned int GameRuleManager::addLevelGenerationOptions(LevelGenerationOptions* options)
{
    std::vector<LevelGenerationOptions*>* generators = mLevelGenerators->getLevelGenerators();
    unsigned int i;
    for (i = 0; i < generators->size(); ++i)
    {
        if (generators->at(i) == options)
        {
            return i;
        }
    }
    generators->push_back(options);
    return generators->size() - 1;
}

void GameRuleManager::readAttributes(DataInputStream* input, std::vector<std::wstring>* names, GameRuleDefinition* definition)
{
    unsigned int count = input->readInt();
    unsigned int i;
    for (i = 0; i < count; ++i)
    {
        unsigned int nameIndex = input->readInt();
        std::wstring value = input->readUTF();
        if (definition != NULL)
        {
            definition->addAttribute(names->at(nameIndex), value);
        }
    }
    definition->onAttributesAdded();
}

void GameRuleManager::readChildren(DataInputStream* input, std::vector<std::wstring>* names, boost::unordered_map<int, ConsoleGameRules::EGameRuleType>* types, GameRuleDefinition* definition)
{
    unsigned int count = input->readInt();
    unsigned int i;
    for (i = 0; i < count; ++i)
    {
        int id = input->readInt();
        ConsoleGameRules::EGameRuleType type = static_cast<ConsoleGameRules::EGameRuleType>(-1);
        boost::unordered_map<int, ConsoleGameRules::EGameRuleType>::iterator found = types->find(id);
        if (found != types->end())
        {
            type = found->second;
        }
        GameRuleDefinition* child = definition == NULL ? NULL : definition->addChild(type);
        readAttributes(input, names, child);
        readChildren(input, names, types, child);
    }
}

unsigned int GameRuleManager::readRuleFile(LevelGenerationOptions* options, unsigned char* data, unsigned int size, StringTable* table)
{
    arrayWithLength<unsigned char> inputData;
    inputData.data = data;
    inputData.length = size;
    ByteArrayInputStream bytes(inputData);
    DataInputStream input(&bytes);
    DataInputStream* rules = &input;
    ByteArrayInputStream* expandedBytes = NULL;
    DataInputStream* expandedInput = NULL;
    short version = input.readShort();
    unsigned char compression = 0;
    if (version == 0)
    {
        for (int i = 0; i < 14; ++i) input.readByte();
    }
    else
    {
        compression = input.readByte();
        unsigned int expectedCRC = input.readInt();
        input.readBoolean();
        input.readBoolean();
        input.readBoolean();
        input.readBoolean();
        if (compression != 0)
        {
            unsigned int expandedSize = input.readInt();
            unsigned int compressedSize = input.readInt();
            arrayWithLength<unsigned char> compressed(compressedSize, true);
            input.read(compressed);
            options->setCRCIsValid(CRC(compressed.data, compressed.length) == expectedCRC);
            arrayWithLength<unsigned char> expanded(expandedSize, true);
            if (compression == 1)
            {
                Compression::getCompression()->Decompress(expanded.data, &expanded.length, compressed.data, compressed.length);
            }
            else
            {
                Compression::getCompression()->DecompressLZXRLE(expanded.data, &expanded.length, compressed.data, compressed.length);
            }
            delete[] compressed.data;
            compressed.data = NULL;
            expandedBytes = new ByteArrayInputStream(expanded);
            expandedInput = new DataInputStream(expandedBytes);
            rules = expandedInput;
        }
    }
    unsigned int stringCount = rules->readInt();
    std::vector<std::wstring> names;
    names.reserve(stringCount);
    for (unsigned int i = 0; i < stringCount; ++i)
    {
        names.push_back(rules->readUTF());
    }
    boost::unordered_map<int, ConsoleGameRules::EGameRuleType> types;
    for (unsigned int type = 0; type < 77; ++type)
    {
        for (unsigned int name = 0; name < names.size(); ++name)
        {
            if (names[name] == gGameRuleTypeNames[type])
            {
                types[name] = static_cast<ConsoleGameRules::EGameRuleType>(type);
                break;
            }
        }
    }
    unsigned int schematicCount = rules->readInt();
    for (unsigned int schematic = 0; schematic < schematicCount; ++schematic)
    {
        std::wstring name = rules->readUTF();
        unsigned int schematicSize = rules->readInt();
        arrayWithLength<unsigned char> schematicData(schematicSize, true);
        rules->read(schematicData);
        options->loadSchematicFile(name, schematicData.data, schematicData.length);
    }
    unsigned int rootCount = rules->readInt();
    for (unsigned int root = 0; root < rootCount; ++root)
    {
        int nameIndex = rules->readInt();
        ConsoleGameRules::EGameRuleType type = static_cast<ConsoleGameRules::EGameRuleType>(-1);
        boost::unordered_map<int, ConsoleGameRules::EGameRuleType>::iterator found = types.find(nameIndex);
        if (found != types.end()) type = found->second;
        GameRuleDefinition* definition = NULL;
        if (type == static_cast<ConsoleGameRules::EGameRuleType>(1))
        {
            addLevelGenerationOptions(options);
            options->loadStringTable(table);
            definition = options;
        }
        else if (type == static_cast<ConsoleGameRules::EGameRuleType>(16))
        {
            LevelRuleset* ruleset = new LevelRuleset();
            options->setRequiredGameRules(ruleset);
            ruleset->setGenerationOptions(options);
            ruleset->loadStringTable(table);
            mLevelRules->addLevelRule(L"", ruleset);
            definition = ruleset;
        }
        readAttributes(rules, &names, definition);
        readChildren(rules, &names, &types, definition);
    }
    if (expandedInput != NULL)
    {
        expandedInput->close();
        delete expandedInput;
        delete expandedBytes;
    }
    input.close();
    return 1;
}

void GameRuleManager::loadGameRules(DLCPack* pack)
{
    StringTable* table = NULL;
    if (pack->doesPackContainFile(DLCManager::eDLCType_Localisation, std::wstring(L"languages_loc"), true, true))
    {
        DLCLocalisationFile* localisation = static_cast<DLCLocalisationFile*>(pack->getFile(DLCManager::eDLCType_Localisation, std::wstring(L"languages_loc"), true, true));
        table = localisation->getStringTable();
    }
    int count = pack->getDLCItemsCount(DLCManager::eDLCType_GameRulesHeader);
    for (int i = 0; i < count; ++i)
    {
        DLCGameRulesHeader* header = static_cast<DLCGameRulesHeader*>(pack->getFile(DLCManager::eDLCType_GameRulesHeader, i, true, true));
        unsigned int size = 0;
        unsigned char* data = header->getData(size);
        LevelGenerationOptions* options = new LevelGenerationOptions(pack);
        options->setGrSource(&header->m_gameRulesSource);
        options->setSrc(LevelGenerationOptions::eSrc_DLC);
        readRuleFile(options, data, size, table);
        header->m_levelGenerationOptions = options;
    }
    count = pack->getDLCItemsCount(DLCManager::eDLCType_GameRules);
    for (int i = 0; i < count; ++i)
    {
        DLCFile* file = pack->getFile(DLCManager::eDLCType_GameRules, i, true, true);
        unsigned int size = 0;
        unsigned char* data = file->getData(size);
        LevelGenerationOptions* options = new LevelGenerationOptions(pack);
        options->setGrSource(new GameRulesSource());
        options->setSrc(LevelGenerationOptions::eSrc_Local);
        readRuleFile(options, data, size, table);
        options->setLoadedData();
    }
}

void GameRuleManager::loadGameRules(LevelGenerationOptions* options, unsigned char* data, unsigned int size)
{
    arrayWithLength<unsigned char> inputData;
    inputData.data = data;
    inputData.length = size;
    ByteArrayInputStream bytes(inputData);
    DataInputStream input(&bytes);
    input.readShort();
    for (int i = 0; i < 8; ++i) input.readByte();
    input.readByte();
    unsigned int expandedSize = input.readInt();
    unsigned int compressedSize = input.readInt();
    arrayWithLength<unsigned char> compressed(compressedSize, true);
    input.read(compressed);
    arrayWithLength<unsigned char> expanded(expandedSize, true);
    Compression::getCompression()->DecompressLZXRLE(expanded.data, &expanded.length, compressed.data, compressed.length);
    input.close();
    bytes.close();
    delete[] compressed.data;
    compressed.data = NULL;
    ByteArrayInputStream rulesBytes(expanded);
    DataInputStream rules(&rulesBytes);
    unsigned int tableSize = rules.readInt();
    arrayWithLength<unsigned char> tableData(tableSize, true);
    rules.read(tableData);
    StringTable* table = new StringTable(tableData, true, true);
    unsigned int remaining = expanded.length - tableSize;
    arrayWithLength<unsigned char> ruleData(remaining, true);
    rules.read(ruleData);
    if (readRuleFile(options, ruleData.data, ruleData.length, table) == 0)
    {
        delete options;
        delete table;
    }
    else
    {
        options->setSrc(LevelGenerationOptions::eSrc_Save);
        setLevelGenerationOptions(options);
    }
    rules.close();
    rulesBytes.close();
}

LevelGenerationOptions* GameRuleManager::loadGameRules(unsigned char* data, unsigned int size)
{
    LevelGenerationOptions* options = new LevelGenerationOptions(NULL);
    options->setGrSource(new GameRulesSource());
    options->setSrc(LevelGenerationOptions::eSrc_Save);
    loadGameRules(options, data, size);
    options->setLoadedData();
    return options;
}

LevelGenerationOptions* GameRuleManager::getLevelGenerationOptions()
{
    return mLevelGenerationOptions;
}

void GameRuleManager::writeRuleFile(DataOutputStream* output)
{
    boost::unordered_map<int, int> tags;
    boost::unordered_set<int> tagTypes;
    boost::unordered_set<int> attributes;
    output->writeShort(2);
    output->writeByte(0);
    for (int i = 0; i < 8; ++i) output->writeBoolean(false);
    mLevelGenerationOptions->findDistinctTagsAndAttributes(tagTypes, attributes);
    mGameRuleDefinitions->findDistinctTagsAndAttributes(tagTypes, attributes);
    output->writeInt(tagTypes.size() + attributes.size());
    int key = 0;
    for (int type = 0; type < 77; ++type)
    {
        if (tagTypes.find(type) != tagTypes.end())
        {
            output->writeUTF(gGameRuleTypeNames[type]);
            tags[type] = key++;
        }
    }
    for (int attribute = 0; attribute < 91; ++attribute)
    {
        if (attributes.find(attribute) != attributes.end())
        {
            output->writeUTF(gGameRuleAttributeNames[attribute]);
            tags[attribute + 77] = key++;
        }
    }
    LevelGenerationOptions::SchematicFileMap* schematics = mLevelGenerationOptions->getUnfinishedSchematicFiles();
    output->writeInt(schematics->size());
    LevelGenerationOptions::SchematicFileMap::iterator schematic;
    for (schematic = schematics->begin(); schematic != schematics->end(); ++schematic)
    {
        ByteArrayOutputStream schematicBytes;
        DataOutputStream schematicOutput(&schematicBytes);
        schematic->second->save(&schematicOutput);
        output->writeUTF(schematic->first);
        output->writeInt(schematicBytes.mCount);
        arrayWithLength<unsigned char> schematicData;
        schematicData.data = schematicBytes.mBuffer.data;
        schematicData.length = schematicBytes.mCount;
        output->write(schematicData);
        schematicOutput.close();
        schematicBytes.close();
    }
    delete schematics;
    output->writeInt(2);
    mLevelGenerationOptions->write(output, &tags);
    mGameRuleDefinitions->write(output, &tags);
}

void GameRuleManager::saveGameRules(unsigned char** data, unsigned int* size)
{
    if (mGameRuleDefinitions == NULL && mLevelGenerationOptions == NULL)
    {
        *data = NULL;
        *size = 0;
        return;
    }
    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeShort(1);
    for (int i = 0; i < 8; ++i) output.writeByte(0);
    output.writeByte(3);
    ByteArrayOutputStream rulesBytes;
    DataOutputStream rules(&rulesBytes);
    if (mGameRuleDefinitions == NULL)
    {
        rules.writeInt(0);
        rules.writeInt(2);
        rules.writeByte(0);
        rules.writeByte(0);
        rules.writeByte(0);
        rules.writeInt(0);
        rules.writeInt(0);
        rules.writeInt(0);
    }
    else
    {
        StringTable* table = mGameRuleDefinitions->getStringTable();
        if (table != NULL)
        {
            unsigned char* stringData = NULL;
            unsigned int stringSize = 0;
            table->getData(&stringData, &stringSize);
            rules.writeInt(stringSize);
            arrayWithLength<unsigned char> tableData;
            tableData.data = stringData;
            tableData.length = stringSize;
            rules.write(tableData);
            writeRuleFile(&rules);
        }
    }
    unsigned int sourceSize = rulesBytes.mCount;
    arrayWithLength<unsigned char> compressed(sourceSize, true);
    Compression::getCompression()->CompressLZXRLE(compressed.data, &compressed.length, rulesBytes.mBuffer.data, sourceSize, NULL);
    output.writeInt(compressed.length);
    output.writeInt(sourceSize);
    output.write(compressed);
    delete[] compressed.data;
    compressed.data = NULL;
    rules.close();
    rulesBytes.close();
    *size = bytes.mCount;
    *data = bytes.mBuffer.data;
    bytes.mBuffer.data = NULL;
    output.close();
    bytes.close();
}

void GameRuleManager::processSchematics(LevelChunk* chunk)
{
    if (mLevelGenerationOptions != NULL) mLevelGenerationOptions->processSchematics(chunk);
}

void GameRuleManager::processSchematicsLighting(LevelChunk* chunk)
{
    if (mLevelGenerationOptions != NULL) mLevelGenerationOptions->processSchematicsLighting(chunk);
}

unsigned int GameRuleManager::loadGameRulesPack(File* file)
{
    if (!file->exists()) return 0;
    DLCPack* pack = new DLCPack(L"", 0xffffffffU);
    unsigned int count = 0;
    if (!g_DLCManager.readDLCDataFile(count, file->getPath(), pack, 0, false))
    {
        delete pack;
        return 0;
    }
    g_DLCManager.addPack(pack);
    return 1;
}

void GameRuleManager::loadDefaultGameRules()
{
    std::vector<LevelGenerationOptions*>* options = mLevelGenerators->getLevelGenerators();
    unsigned int index = options->size();
    std::wstring root(L"WiiU_Lobby_");
    File file(root + L"Lobby_pck");
    if (loadGameRulesPack(&file) != 0)
    {
        options->at(index)->setBaseSaveRoot(root);
        options->at(index)->setIsGameModeLobby(true);
    }
}

LevelGenerators* GameRuleManager::getLevelGenerators()
{
    return mLevelGenerators;
}

LevelRuleset* GameRuleManager::getGameRuleDefinitions()
{
    return mGameRuleDefinitions;
}

const wchar_t* GameRuleManager::GetGameRulesString(const std::wstring& key)
{
    if (mGameRuleDefinitions != NULL && !key.empty()) return mGameRuleDefinitions->getString(key);
    return L"target";
}
