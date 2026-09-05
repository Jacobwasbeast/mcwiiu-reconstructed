#include "game/world/chunk/OldChunkStorage.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/NbtIo.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"

extern unsigned int TlsAlloc();
extern void TlsSetValue(unsigned int key, void* value);
extern void* TlsGetValue(unsigned int key);

static unsigned int gOldChunkStorageTlsKey = 0;
static OldChunkStorage::ThreadStorage* gOldChunkStorageDefault = NULL;

OldChunkStorage::ThreadStorage::ThreadStorage()
    : blockData(NULL), blockDataSize(0), data(NULL), dataSize(0),
      skyLightData(NULL), skyLightDataSize(0), blockLightData(NULL), blockLightDataSize(0)
{
    blockData = new unsigned char[0x10000]();
    blockDataSize = 0x10000;
    data = new unsigned char[0x8000]();
    dataSize = 0x8000;
    skyLightData = new unsigned char[0x8000]();
    skyLightDataSize = 0x8000;
    blockLightData = new unsigned char[0x8000]();
    blockLightDataSize = 0x8000;
}

OldChunkStorage::ThreadStorage::~ThreadStorage()
{
    delete[] blockData;
    delete[] data;
    delete[] skyLightData;
    delete[] blockLightData;
}

void OldChunkStorage::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();
    if (gOldChunkStorageDefault == NULL)
    {
        gOldChunkStorageTlsKey = TlsAlloc();
        gOldChunkStorageDefault = storage;
    }
    TlsSetValue(gOldChunkStorageTlsKey, storage);
}

void OldChunkStorage::UseDefaultThreadStorage()
{
    TlsSetValue(gOldChunkStorageTlsKey, gOldChunkStorageDefault);
}

void OldChunkStorage::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(gOldChunkStorageTlsKey));
    if (storage != gOldChunkStorageDefault)
        delete storage;
}

OldChunkStorage::OldChunkStorage(File file, bool createValue)
    : directory(), create(createValue)
{
    directory = file;
}

OldChunkStorage::~OldChunkStorage()
{
}

File OldChunkStorage::getFile(int x, int z)
{
    std::wstring region(L"region");
    File regionDirectory(directory, region);
    if (!regionDirectory.exists())
    {
        if (!create)
            return File(L"Entities");
        regionDirectory.mkdir();
    }
    wchar_t name[256];
    std::swprintf(name, 256, L"r.%d.%d.mca", x >> 5, z >> 5);
    File result(regionDirectory, std::wstring(name));
    if (!result.exists() && !create)
        return File(L"Entities");
    return result;
}

void OldChunkStorage::swapBlockID(LevelChunk* chunk, int x, int y, int z, int oldId, int newId)
{
    unsigned short element = static_cast<unsigned short>((chunk->getBlockId(x, y, z) << 4) | (chunk->getData(x, y, z) & 0xf));
    int id = static_cast<int>(element) >> 4;
    if (id == oldId || id == newId)
    {
        int replacement = id == oldId ? newId : oldId;
        chunk->setElement(x, y, z, static_cast<unsigned short>((replacement & 0xfff) << 4 | (element & 0xf)));
    }
}

void OldChunkStorage::fixBlockIDs(LevelChunk* chunk)
{
    int y;
    int z;
    int x;
    for (y = 0; y < 256; ++y)
        for (z = 0; z < 16; ++z)
            for (x = 0; x < 16; ++x)
            {
                swapBlockID(chunk, x, y, z, 0xa1, 0xa2);
                swapBlockID(chunk, x, y, z, 0xba, 0xbb);
                swapBlockID(chunk, x, y, z, 0xbf, 0xc0);
            }
}

boost::shared_ptr<Entity> OldChunkStorage::loadStaticEntity(CompoundTag* tag, Level* level)
{
    return EntityIO::newEntity(tag, level);
}

boost::shared_ptr<Entity> OldChunkStorage::loadAndAddEntity(CompoundTag* tag, Level* level, LevelChunk* chunk)
{
    boost::shared_ptr<Entity> entity = loadStaticEntity(tag, level);
    if (entity)
    {
        if (tag->contains(L"Passengers", 9))
        {
            ListTag* passengers = tag->getList(L"Passengers", 10);
            if (passengers != NULL)
            {
                int i;
                for (i = 0; i < passengers->size(); ++i)
                    loadAndAddEntity(passengers->getCompound(i), level, chunk);
            }
        }
        chunk->addEntity(entity);
    }
    return entity;
}

void OldChunkStorage::loadEntities(LevelChunk* chunk, Level* level, CompoundTag* tag)
{
    if (tag->contains(L"Entities"))
    {
        ListTag* entities = tag->getList(L"Entities", 10);
        if (entities != NULL)
        {
            int i;
            for (i = 0; i < entities->size(); ++i)
                loadAndAddEntity(entities->getCompound(i), level, chunk);
            chunk->setLastSaveHadEntities(true);
        }
    }
    if (tag->contains(L"TileEntities"))
    {
        ListTag* entities = tag->getList(L"TileEntities", 10);
        if (entities != NULL)
        {
            int i;
            for (i = 0; i < entities->size(); ++i)
            {
                boost::shared_ptr<BlockEntity> blockEntity = BlockEntity::loadStatic(level, entities->getCompound(i));
                if (blockEntity)
                    chunk->addBlockEntity(blockEntity);
            }
        }
    }
}

LevelChunk* OldChunkStorage::load(Level* level, CompoundTag* tag, LevelChunk* chunk)
{
    CompoundTag* levelTag = tag->getCompound(L"Level");
    if (levelTag == NULL)
        return NULL;
    int x = levelTag->getInt(L"xPos");
    int z = levelTag->getInt(L"zPos");
    if (chunk == NULL)
        chunk = new LevelChunk(level, x, z);
    if (levelTag->contains(L"InhabitedTime"))
        chunk->setInhabitedTime(levelTag->getLong(L"InhabitedTime"));
    if (levelTag->contains(L"TerrainPopulated"))
        chunk->setTerrainPopulated(levelTag->getByte(L"TerrainPopulated"));
    if (levelTag->contains(L"Biomes"))
        chunk->setBiomes(levelTag->getByteArray(L"Biomes"));
    if (levelTag->contains(L"Sections"))
    {
        ListTag* sections = levelTag->getList(L"Sections", 10);
        (void)sections;
    }
    if (levelTag->contains(L"V"))
        chunk->setChunkVersion(levelTag->getInt(L"V"));
    chunk->recalcMinAndMaxHeights();
    loadEntities(chunk, level, levelTag);
    fixBlockIDs(chunk);
    fixCauldronData(chunk);
    return chunk;
}

LevelChunk* OldChunkStorage::load(Level* level, int x, int z, LevelChunk* chunk)
{
    File file = getFile(x, z);
    if (!file.exists())
        return NULL;
    FileInputStream input(file);
    CompoundTag* root = NbtIo::readCompressed(&input);
    if (root == NULL)
        return NULL;
    LevelChunk* result = load(level, root, chunk);
    delete root;
    return result;
}

void OldChunkStorage::save(Level* level, LevelChunk* chunk)
{
    (void)level;
    (void)chunk;
}

bool OldChunkStorage::saveEntities(LevelChunk* chunk, Level* level, CompoundTag* tag)
{
    (void)level;
    ListTag* entitiesTag = new ListTag(10);
    std::vector<boost::shared_ptr<Entity> > entities;
    chunk->getEntities(boost::shared_ptr<Entity>(), NULL, entities, NULL);
    std::vector<boost::shared_ptr<Entity> >::iterator entity;
    for (entity = entities.begin(); entity != entities.end(); ++entity)
    {
        CompoundTag* entityTag = new CompoundTag();
        if ((*entity)->save(entityTag))
            entitiesTag->add(entityTag);
        else
            delete entityTag;
    }
    tag->put(L"Entities", entitiesTag);

    ListTag* blockEntitiesTag = new ListTag(10);
    LevelChunk::BlockEntityMap* blockEntities = chunk->getBlockEntities();
    LevelChunk::BlockEntityMap::iterator blockEntity;
    for (blockEntity = blockEntities->begin(); blockEntity != blockEntities->end(); ++blockEntity)
    {
        CompoundTag* blockEntityTag = new CompoundTag();
        blockEntity->second->save(blockEntityTag);
        blockEntitiesTag->add(blockEntityTag);
    }
    tag->put(L"TileEntities", blockEntitiesTag);
    return true;
}

void OldChunkStorage::save(LevelChunk* chunk, Level* level, DataOutputStream* stream)
{
    CompoundTag tag;
    CompoundTag* levelTag = new CompoundTag();
    levelTag->putInt(L"xPos", chunk->getPos().x);
    levelTag->putInt(L"zPos", chunk->getPos().z);
    levelTag->putLong(L"InhabitedTime", chunk->getInhabitedTime());
    saveEntities(chunk, level, levelTag);
    tag.put(L"Level", levelTag);
    NbtIo::write(&tag, stream);
}

boost::shared_ptr<Entity> OldChunkStorage::loadEntity(CompoundTag* tag, Level* level, double x, double y, double z, bool addToWorld)
{
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, level);
    if (entity)
    {
        entity->setPos(x, y, z);
        if (addToWorld)
            addEntityToWorld(entity, level);
    }
    return entity;
}

void OldChunkStorage::addEntityToWorld(boost::shared_ptr<Entity> entity, Level* level)
{
    if (entity)
        level->addEntity(entity);
}

boost::shared_ptr<Entity> OldChunkStorage::loadEntity(CompoundTag* tag, Level* level, bool addToWorld)
{
    return loadEntity(tag, level, 0.0, 0.0, 0.0, addToWorld);
}

void OldChunkStorage::fixCauldronData(LevelChunk* chunk)
{
    int x;
    int y;
    int z;
    for (x = 0; x < 16; ++x)
        for (z = 0; z < 16; ++z)
            for (y = 0; y < 256; ++y)
            {
                const BlockState* state = chunk->getBlockState(x, y, z);
                if (state != NULL && Block::getElementId(state->getBlock()) == 0x76)
                {
                    int data = chunk->getData(x, y, z) * 2;
                    if (data < 0) data = 0;
                    if (data > 6) data = 6;
                    chunk->setBlockAndData(x, y, z, 0x76, data, false, false);
                }
            }
}

LevelChunk* OldChunkStorage::load(Level* level, DataInputStream* stream, DataFixerUpper* fixer, LevelChunk* chunk)
{
    CompoundTag* tag = NbtIo::read(stream);
    if (tag == NULL)
        return NULL;
    if (fixer != NULL)
        tag = fixer->updateData(DataFixTypes::eDataFixTypes_Chunk, tag);
    LevelChunk* result = load(level, tag, chunk);
    delete tag;
    return result;
}

void OldChunkStorage::tick()
{
}

void OldChunkStorage::flush()
{
}

void OldChunkStorage::saveEntities(Level* level, LevelChunk* chunk)
{
    (void)level;
    (void)chunk;
}
