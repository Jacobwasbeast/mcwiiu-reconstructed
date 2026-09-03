#ifndef GAME_WORLD_CHUNK_CHUNKGENERATOR_HPP
#define GAME_WORLD_CHUNK_CHUNKGENERATOR_HPP

class LevelChunk;

class ChunkGenerator
{
public:
    virtual ~ChunkGenerator();
    virtual void lightChunk(LevelChunk* chunk);
};

#endif
