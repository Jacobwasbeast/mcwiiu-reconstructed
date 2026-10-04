#include "game/world/chunk/McRegionChunkStorage.hpp"
#include <cstdio>
#include <deque>
#include "game/core/CriticalSection.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/RegionFile.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/ENTITIESONLY_DataWalker.hpp"
#include "game/world/LEVELDATA_DataWalker.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"

void SetThreadName(unsigned int threadId, const char* name);

static OSFastMutex sSaveMutex;
static std::deque<DataOutputStream*> sSaveQueue;
static int sActiveSaves = 0;
static C4JThreadImpl* sSaveThreads[3] = { NULL, NULL, NULL };

McRegionChunkStorage::McRegionChunkStorage(ConsoleSaveFile* saveFile,
    const std::wstring& directory, DataFixerUpper* fixer)
    : mDirectory(directory), mRegionData()
{
    mSaveFile = saveFile;
    mFixer = fixer;
    if (directory == L"DIM-1r")
    {
        mSaveFile->createFile(ConsoleSavePath(L"DIM-1r.-1.-1.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM-1r.0.-1.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM-1r.0.0.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM-1r.-1.0.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM1/r.-1.-1.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM1/r.0.-1.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM1/r.0.0.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"DIM1/r.-1.0.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"r.-1.-1.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"r.0.-1.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"r.0.0.mcr"));
        mSaveFile->createFile(ConsoleSavePath(L"r.-1.0.mcr"));
    }
}

McRegionChunkStorage::~McRegionChunkStorage()
{
    for (RegionDataMap::iterator it = mRegionData.begin(); it != mRegionData.end(); ++it)
        delete it->second.data;
}

void McRegionChunkStorage::runSaveThreadProc(void*)
{
    Compression::CreateNewThreadStorage();
    RegionFile::CreateNewThreadStorage();
    unsigned int previousQueueSize = 0;
    for (;;)
    {
        DataOutputStream* stream = NULL;
        unsigned int queueSize = 0;
        if (TryEnterCriticalSection(&sSaveMutex))
        {
            queueSize = (unsigned int)sSaveQueue.size();
            if (queueSize != 0)
            {
                stream = sSaveQueue.front();
                sSaveQueue.pop_front();
            }
            ++sActiveSaves;
            LeaveCriticalSection(&sSaveMutex);
            if (stream != NULL)
            {
                PIXBeginNamedEvent(0.0f, "Saving chunk");
                stream->close();
                stream->deleteChildStream();
                PIXEndNamedEvent();
                delete stream;
            }
            EnterCriticalSection(&sSaveMutex);
            --sActiveSaves;
            LeaveCriticalSection(&sSaveMutex);
            previousQueueSize = queueSize;
        }
        Sleep(previousQueueSize > 1 ? 1 : 100);
    }
}

void McRegionChunkStorage::staticCtor()
{
    InitializeCriticalSectionAndSpinCount(&sSaveMutex, 0x1400);
    char name[256];
    int i;
    for (i = 0; i < 3; ++i)
    {
        std::sprintf(name, "McRegion Save thread %d\n", i);
        SetThreadName(0, name);
        sSaveThreads[i] = new C4JThreadImpl(
            (int (*)(void*))McRegionChunkStorage::runSaveThreadProc, NULL, name, 0);
        sSaveThreads[i]->SetProcessor(0);
        sSaveThreads[i]->Run();
    }
}

LevelChunk* McRegionChunkStorage::load(Level* level, int x, int z, LevelChunk* chunk)
{
    bool newSaveFormat = false;
    DataInputStream* stream = gRegionFileCache._getChunkDataInputStream(
        mSaveFile, mDirectory, x, z, &newSaveFormat);
    if (mSaveFile->getOriginalSaveVersion() < 8 && !newSaveFormat)
    {
        if (stream == NULL)
            return NULL;
        char message[260];
        MemSect(8);
        CompoundTag* tag = mFixer->updateData(DataFixTypes::eDataFixTypes_Chunk, NbtIo::read(stream));
        MemSect(0);
        stream->deleteChildStream();
        delete stream;
        if (!tag->contains(L"Level"))
        {
            std::sprintf(message, "Chunk file at %d, %d is missing level data, skipping\n", x, z);
            delete tag;
            return NULL;
        }
        if (!tag->getCompound(L"Level")->contains(L"Blocks"))
        {
            std::sprintf(message, "Chunk file at %d, %d is missing block data, skipping\n", x, z);
            delete tag;
            return NULL;
        }
        MemSect(9);
        chunk = OldChunkStorage::load(level, tag->getCompound(L"Level"), chunk);
        MemSect(0);
        if (!chunk->isAt(x, z))
        {
            std::sprintf(message, "Chunk file at %d, %d is in the wrong location; relocating. Expected %d, %d, got %d, %d\n",
                x, z, x, z, chunk->x, chunk->z);
            delete chunk;
            delete tag;
            return NULL;
        }
        delete tag;
    }
    else if (stream != NULL)
    {
        MemSect(9);
        chunk = OldChunkStorage::load(level, stream, mFixer, NULL);
        loadEntities(level, chunk);
        MemSect(0);
        stream->deleteChildStream();
        delete stream;
    }
    return chunk;
}

void McRegionChunkStorage::save(Level* level, LevelChunk* chunk)
{
    level->checkSession();
    MemSect(0x1e);
    PIXBeginNamedEvent(0.0f, "Getting output stream\n");
    DataOutputStream* stream = gRegionFileCache._getChunkDataOutputStream(
        mSaveFile, mDirectory, chunk->x, chunk->z, true);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Writing chunk data");
    OldChunkStorage::save(chunk, level, stream);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Updating chunk queue");
    EnterCriticalSection(&sSaveMutex);
    sSaveQueue.push_back(stream);
    LeaveCriticalSection(&sSaveMutex);
    PIXEndNamedEvent();
    MemSect(0);
    level->getLevelData()->setSizeOnDisk(mSaveFile->getSizeOnDisk());
}

void McRegionChunkStorage::saveEntities(Level*, LevelChunk*)
{
}

void McRegionChunkStorage::loadEntities(Level*, LevelChunk*)
{
}

void McRegionChunkStorage::tick()
{
    mSaveFile->tick();
}

void McRegionChunkStorage::flush()
{
}

void McRegionChunkStorage::addWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Chunk, new LEVELDATA_DataWalker());
    fixer->addWalker(DataFixTypes::eDataFixTypes_Chunk, new ENTITIESONLY_DataWalker());
}

void McRegionChunkStorage::WaitForAllSaves()
{
    unsigned int count;
    EnterCriticalSection(&sSaveMutex);
    count = (unsigned int)sSaveQueue.size();
    LeaveCriticalSection(&sSaveMutex);
    while (count != 0)
    {
        Sleep(10);
        EnterCriticalSection(&sSaveMutex);
        count = (unsigned int)sSaveQueue.size();
        LeaveCriticalSection(&sSaveMutex);
    }
    int active;
    EnterCriticalSection(&sSaveMutex);
    active = sActiveSaves;
    LeaveCriticalSection(&sSaveMutex);
    while (active > 0)
    {
        Sleep(10);
        EnterCriticalSection(&sSaveMutex);
        active = sActiveSaves;
        LeaveCriticalSection(&sSaveMutex);
    }
}

void McRegionChunkStorage::WaitForAll()
{
    WaitForAllSaves();
}

void McRegionChunkStorage::WaitForSaves()
{
    unsigned int count;
    EnterCriticalSection(&sSaveMutex);
    count = (unsigned int)sSaveQueue.size();
    LeaveCriticalSection(&sSaveMutex);
    if (count > 12)
    {
        do
        {
            Sleep(10);
            EnterCriticalSection(&sSaveMutex);
            count = (unsigned int)sSaveQueue.size();
            LeaveCriticalSection(&sSaveMutex);
        }
        while (count > 6);
    }
}

void McRegionChunkStorage::WaitIfTooManyQueuedChunks()
{
    WaitForSaves();
}
