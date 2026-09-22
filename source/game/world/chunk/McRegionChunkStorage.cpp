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

void MemSect(int section);
void Sleep(unsigned int milliseconds);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
void SetThreadName(unsigned int threadId, const char* name);

static OSFastMutex sSaveMutex;
static std::deque<DataOutputStream*> sSaveQueue;
static unsigned int sActiveSaves = 0;
static C4JThreadImpl* sSaveThreads[3] = { NULL, NULL, NULL };

McRegionChunkStorage::McRegionChunkStorage(ConsoleSaveFile* saveFile,
    const std::wstring& directory, DataFixerUpper* fixer)
    : mFixer(fixer), mDirectory(directory), mSaveFile(saveFile), mRegionData()
{
    if (directory == L"DIM-1r")
    {
        const wchar_t* paths[] = {
            L"DIM-1r.-1.-1.mcr", L"DIM-1r.0.-1.mcr", L"DIM-1r.0.0.mcr",
            L"DIM-1r.-1.0.mcr", L"DIM1/r.-1.-1.mcr", L"DIM1/r.0.-1.mcr",
            L"DIM1/r.0.0.mcr", L"DIM1/r.-1.0.mcr", L"r.-1.-1.mcr",
            L"r.0.-1.mcr", L"r.0.0.mcr", L"r.-1.0.mcr" };
        int i;
        for (i = 0; i < 12; ++i)
            saveFile->createFile(ConsoleSavePath(paths[i]));
    }
}

McRegionChunkStorage::~McRegionChunkStorage()
{
    for (RegionDataMap::iterator it = mRegionData.begin(); it != mRegionData.end(); ++it)
        delete[] it->second.data;
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
                stream->flush();
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
        std::sprintf(name, "McRegion_Save_thread_%d", i);
        SetThreadName(0, name);
        sSaveThreads[i] = new C4JThreadImpl(
            (int (*)(void*))McRegionChunkStorage::runSaveThreadProc, NULL, name, 0);
        sSaveThreads[i]->SetDeleteOnExit(false);
        sSaveThreads[i]->Run();
    }
}

LevelChunk* McRegionChunkStorage::load(Level* level, int x, int z, LevelChunk* chunk)
{
    bool newSaveFormat = false;
    DataInputStream* stream = gRegionFileCache._getChunkDataInputStream(
        mSaveFile, mDirectory, x, z, &newSaveFormat);
    if (mSaveFile->getOriginalSaveVersion() > 7 || newSaveFormat)
    {
        if (stream == NULL)
            return chunk;
        MemSect(9);
        LevelChunk* result = OldChunkStorage::load(level, stream, mFixer, chunk);
        MemSect(0);
        stream->deleteChildStream();
        delete stream;
        return result;
    }
    if (stream == NULL)
        return NULL;
    MemSect(8);
    CompoundTag* tag = NbtIo::read(stream);
    tag = mFixer->updateData(DataFixTypes::eDataFixTypes_Chunk, tag);
    MemSect(0);
    stream->deleteChildStream();
    delete stream;
    if (tag == NULL)
        return NULL;
    LevelChunk* result = NULL;
    if (tag->contains(L"Level", 10))
    {
        CompoundTag* levelTag = tag->getCompound(L"Level");
        if (levelTag != NULL && levelTag->contains(L"Blocks", 7))
        {
            MemSect(9);
            result = OldChunkStorage::load(level, levelTag, chunk);
            MemSect(0);
        }
    }
    delete tag;
    if (result != NULL && !result->isAt(x, z))
    {
        delete result;
        return NULL;
    }
    return result;
}

void McRegionChunkStorage::save(Level* level, LevelChunk* chunk)
{
    level->checkSession();
    MemSect(0x1e);
    PIXBeginNamedEvent(0.0f, "Getting output stream");
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
    EnterCriticalSection(&sSaveMutex);
    count = sActiveSaves;
    LeaveCriticalSection(&sSaveMutex);
    while (count != 0)
    {
        Sleep(10);
        EnterCriticalSection(&sSaveMutex);
        count = sActiveSaves;
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
