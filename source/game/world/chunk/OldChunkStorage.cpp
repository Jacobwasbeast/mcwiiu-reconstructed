#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/block/BlockId.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/Mth.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/TickNextTickData.hpp"
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
#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int gOldChunkStorageTlsKey = 0;
static OldChunkStorage::ThreadStorage* gOldChunkStorageDefault = NULL;
static OSFastMutex gOldChunkStorageEntityMutex;

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
    unsigned short element = chunk->getElement(x, y, z);
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
        chunk->addEntity(entity);
        if (tag->contains(L"Riding", Tag::TAG_LIST))
        {
            ListTag* passengers = tag->getList(L"Riding", Tag::TAG_COMPOUND);
            if (passengers != NULL)
            {
                int i;
                for (i = 0; i < passengers->size(); ++i)
                {
                    boost::shared_ptr<Entity> passenger = loadAndAddEntity(passengers->getCompound(i), level, chunk);
                    if (passenger)
                        passenger->startRiding(entity, true);
                }
            }
        }
    }
    return entity;
}

void OldChunkStorage::loadEntities(LevelChunk* chunk, Level* level, CompoundTag* tag)
{
    if (tag->contains(L"Entities"))
    {
        ListTag* entities = tag->getList(L"Entities", Tag::TAG_COMPOUND);
        if (entities != NULL)
        {
            int i;
            for (i = 0; i < entities->size(); ++i)
            {
                loadAndAddEntity(entities->getCompound(i), level, chunk);
                chunk->setLastSaveHadEntities(true);
            }
        }
    }
    if (tag->contains(L"TileEntities"))
    {
        ListTag* entities = tag->getList(L"TileEntities", Tag::TAG_COMPOUND);
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
    int version = 9;
    if (tag->contains(L"version", Tag::TAG_SHORT))
        version = tag->getShort(L"version");
    int x = tag->getInt(L"xPos");
    int z = tag->getInt(L"zPos");
    if (chunk == NULL)
        chunk = new LevelChunk(level, x, z);
    chunk->setChunkVersion(version);
    if (version < 10)
        fixBlockIDs(chunk);
    delete chunk->mElements;
    chunk->mElements = new ElementStorage(false, tag->getByteArray(L"Blocks"), tag->getByteArray(L"Data"), 0);
    chunk->setSkyLightData(tag->getByteArray(L"SkyLight"));
    chunk->setBlockLightData(tag->getByteArray(L"BlockLight"));
    arrayWithLength<unsigned char> heightmap = tag->getByteArray(L"HeightMap");
    delete[] chunk->mHeightmap.data;
    chunk->mHeightmap = arrayWithLength<unsigned char>(heightmap.length, false);
    if (heightmap.length != 0)
        std::memcpy(chunk->mHeightmap.data, heightmap.data, heightmap.length);
    chunk->recalcMinAndMaxHeights();
    if (tag->get(L"TerrainPopulated") == NULL)
    {
        chunk->mChunkFlags = static_cast<unsigned short>(tag->getShort(L"TerrainPopulatedFlags"));
        if ((chunk->mChunkFlags & LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS) == LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS)
            chunk->mChunkFlags |= LevelChunk::TERRAIN_POPULATED_FULL;
    }
    else
    {
        chunk->mChunkFlags = static_cast<unsigned short>(tag->getByte(L"TerrainPopulated"));
        if (static_cast<short>(chunk->mChunkFlags) > 0)
            chunk->mChunkFlags = LevelChunk::TERRAIN_POPULATED_ALL;
    }
    if (tag->contains(L"Biomes"))
    {
        arrayWithLength<unsigned char> biomes = tag->getByteArray(L"Biomes");
        arrayWithLength<unsigned char> copiedBiomes(biomes.length, false);
        if (biomes.length != 0)
            std::memcpy(copiedBiomes.data, biomes.data, biomes.length);
        chunk->setBiomes(copiedBiomes);
    }
    loadEntities(chunk, level, tag);
    if (tag->contains(L"TileTicks"))
    {
        ListTag* ticks = tag->getList(L"TileTicks", Tag::TAG_COMPOUND);
        int i;
        for (i = 0; ticks != NULL && i < ticks->size(); ++i)
        {
            CompoundTag* tick = ticks->getCompound(i);
            level->addToTickNextTick(BlockPos(tick->getInt(L"x"), tick->getInt(L"y"), tick->getInt(L"z")),
                                     Block::byId(tick->getInt(L"i")), tick->getInt(L"t"), tick->getInt(L"p"));
        }
    }
    return chunk;
}

LevelChunk* OldChunkStorage::load(Level* level, int x, int z, LevelChunk* chunk)
{
    File file = getFile(x, z);
    if (!file.exists())
        return NULL;
    FileInputStream input(file);
    CompoundTag* root = NbtIo::readCompressed(&input);
    if (root == NULL || !root->contains(L"Level", Tag::TAG_COMPOUND))
        return NULL;
    CompoundTag* levelTag = root->getCompound(L"Level");
    if (!levelTag->contains(L"Blocks"))
        return NULL;
    LevelChunk* result = load(level, levelTag, chunk);
    if (!result->isAt(x, z))
    {
        levelTag->putInt(L"xPos", x);
        levelTag->putInt(L"zPos", z);
        result = load(level, levelTag, result);
    }
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
    if (!chunk->mLastSaveHadEntities)
        return false;
    chunk->mLastSaveHadEntities = false;
    ListTag* entitiesTag = new ListTag(0);
    EnterCriticalSection(&gOldChunkStorageEntityMutex);
    int section;
    for (section = 0; section < chunk->mEntitySectionCount; ++section)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = chunk->mEntitySections[section];
        std::vector<boost::shared_ptr<Entity> >::iterator entity;
        for (entity = entities->begin(); entity != entities->end(); ++entity)
        {
            chunk->mLastSaveHadEntities = true;
            CompoundTag* entityTag = new CompoundTag();
            if ((*entity)->save(entityTag))
                entitiesTag->add(entityTag);
            else
                delete entityTag;
        }
    }
    LeaveCriticalSection(&gOldChunkStorageEntityMutex);
    tag->put(L"Entities", entitiesTag);
    return chunk->mLastSaveHadEntities;
}

void OldChunkStorage::save(LevelChunk* chunk, Level* level, DataOutputStream* stream)
{
    stream->writeShort(12);
    stream->writeInt(chunk->x);
    stream->writeInt(chunk->z);
    stream->writeLong(level->getGameTime());
    stream->writeLong(chunk->getInhabitedTime());
    chunk->writeCompressedElements(stream);
    chunk->writeCompressedSkyLightData(stream);
    chunk->writeCompressedBlockLightData(stream);
    stream->writeBytes(chunk->getHeightmap());
    stream->writeUnsignedShort(chunk->mChunkFlags);
    stream->writeBytes(chunk->getBiomes());
    CompoundTag* tag = new CompoundTag();
    saveEntities(chunk, level, tag);
    ListTag* blockEntities = new ListTag(0);
    LevelChunk::BlockEntityMap::iterator blockEntity;
    for (blockEntity = chunk->mBlockEntityMap.begin(); blockEntity != chunk->mBlockEntityMap.end(); ++blockEntity)
    {
        CompoundTag* blockEntityTag = new CompoundTag();
        blockEntity->second->save(blockEntityTag);
        blockEntities->add(blockEntityTag);
    }
    tag->put(L"TileEntities", blockEntities);
    std::vector<TickNextTickData>* ticks = level->fetchTicksInChunk(chunk, false);
    if (ticks != NULL)
    {
        long long gameTime = level->getGameTime();
        ListTag* tickTags = new ListTag(0);
        unsigned int i;
        for (i = 0; i < ticks->size(); ++i)
        {
            TickNextTickData tick = ticks->at(i);
            CompoundTag* tickTag = new CompoundTag();
            tickTag->putInt(L"i", Block::getId(tick.getBlock()));
            tickTag->putInt(L"x", tick.pos.x);
            tickTag->putInt(L"y", tick.pos.y);
            tickTag->putInt(L"z", tick.pos.z);
            tickTag->putInt(L"t", static_cast<int>(tick.triggerTick - gameTime));
            tickTags->add(tickTag);
        }
        tag->put(L"TileTicks", tickTags);
        delete ticks;
    }
    NbtIo::write(tag, stream);
    delete tag;
}

boost::shared_ptr<Entity> OldChunkStorage::loadEntity(CompoundTag* tag, Level* level, double x, double y, double z, bool addToWorld)
{
    boost::shared_ptr<Entity> entity = loadStaticEntity(tag, level);
    if (!entity)
        return entity;

    entity->moveTo(x, y, z, entity->mYRot, entity->mXRot);

    if (addToWorld && !level->addEntity(entity))
        return boost::shared_ptr<Entity>();

    if (tag->contains(L"Riding", Tag::TAG_LIST))
    {
        ListTag* passengers = tag->getList(L"Riding", Tag::TAG_COMPOUND);
        int i;
        for (i = 0; i < passengers->size(); ++i)
        {
            boost::shared_ptr<Entity> passenger = loadEntity(passengers->getCompound(i), level, x, y, z, addToWorld);
            if (passenger)
                passenger->startRiding(entity, true);
        }
    }

    return entity;
}

void OldChunkStorage::addEntityToWorld(boost::shared_ptr<Entity> entity, Level* level)
{
    if (!level->addEntity(entity))
        return;

    if (entity->isVehicle())
    {
        std::vector<boost::weak_ptr<Entity> >* passengers = entity->getPassengers();
        std::vector<boost::weak_ptr<Entity> >::iterator it;
        for (it = passengers->begin(); it != passengers->end(); ++it)
            addEntityToWorld(it->lock(), level);
    }
}

boost::shared_ptr<Entity> OldChunkStorage::loadEntity(CompoundTag* tag, Level* level, bool addToWorld)
{
    boost::shared_ptr<Entity> entity = loadStaticEntity(tag, level);
    if (!entity)
        return entity;

    if (addToWorld && !level->addEntity(entity))
        return boost::shared_ptr<Entity>();

    if (tag->contains(L"Riding", Tag::TAG_LIST))
    {
        ListTag* passengers = tag->getList(L"Riding", Tag::TAG_COMPOUND);
        int i;
        for (i = 0; i < passengers->size(); ++i)
        {
            boost::shared_ptr<Entity> passenger = loadEntity(passengers->getCompound(i), level, addToWorld);
            if (passenger)
                passenger->startRiding(entity, true);
        }
    }

    return entity;
}

static void fixCauldronBlock(LevelChunk* chunk, int x, int y, int z)
{
    const BlockState* state = chunk->getBlockState(x, y, z);
    if (Block::getId(state->getBlock()) == BlockId::CAULDRON)
    {
        bool* changed = new bool(false);
        chunk->setData(x, y, z, Mth::clamp(chunk->getData(x, y, z) << 1, 0, 6), 0, changed);
        delete changed;
    }
}

void OldChunkStorage::fixCauldronData(LevelChunk* chunk)
{
    int x;
    int y;
    int z;
    for (x = 0; x < 16; ++x)
        for (z = 0; z < 16; z += 2)
            for (y = 0; y < 256; y += 2)
            {
                fixCauldronBlock(chunk, x, y, z);
                fixCauldronBlock(chunk, x, y + 1, z);
                fixCauldronBlock(chunk, x, y, z + 1);
                fixCauldronBlock(chunk, x, y + 1, z + 1);
            }
}

LevelChunk* OldChunkStorage::load(Level* level, DataInputStream* stream, DataFixerUpper* fixer, LevelChunk* chunk)
{
    int version = stream->readShort();
    int x = stream->readInt();
    int z = stream->readInt();
    stream->readLong();
    if (chunk == NULL)
        chunk = new LevelChunk(level, x, z);
    if (version > 8)
        chunk->setInhabitedTime(stream->readLong());
    chunk->setChunkVersion(version);
    if (version < 12)
        chunk->readCompressedElementsFromBlockAndData(stream);
    else
        chunk->readCompressedElements(stream);
    chunk->readCompressedSkyLightData(stream);
    chunk->readCompressedBlockLightData(stream);
    if (version < 10)
    {
        fixBlockIDs(chunk);
        fixCauldronData(chunk);
    }
    else if (version < 11)
        fixCauldronData(chunk);
    stream->readBytes(chunk->mHeightmap);
    chunk->recalcMinAndMaxHeights();
    chunk->mChunkFlags = static_cast<unsigned short>(stream->readUnsignedShort());
    if ((chunk->mChunkFlags & LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS) == LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS)
        chunk->mChunkFlags |= LevelChunk::TERRAIN_POPULATED_FULL;
    stream->readBytes(chunk->mBiomes);
    chunk->updateBiomePresentFlags();
    CompoundTag* tag = fixer->updateData(DataFixTypes::eDataFixTypes_Chunk, NbtIo::read(stream));
    loadEntities(chunk, level, tag);
    if (tag->contains(L"TileTicks"))
    {
        ListTag* ticks = tag->getList(L"TileTicks", Tag::TAG_COMPOUND);
        int i;
        for (i = 0; ticks != NULL && i < ticks->size(); ++i)
        {
            CompoundTag* tick = ticks->getCompound(i);
            level->addToTickNextTick(BlockPos(tick->getInt(L"x"), tick->getInt(L"y"), tick->getInt(L"z")),
                                     Block::byId(tick->getInt(L"i")), tick->getInt(L"t"), tick->getInt(L"p"));
        }
    }
    delete tag;
    return chunk;
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
