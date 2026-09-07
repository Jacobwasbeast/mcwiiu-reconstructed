#include "game/world/generation/MapGenerator.hpp"

#include "game/core/System.hpp"

MapGenerator::MapGenerator(Random random)
    : mRandom(random)
{
}

MapGenerator::~MapGenerator()
{
    std::vector<GenerationWriteTask*>::iterator task = mGenerationWriteTasks.begin();
    while (task != mGenerationWriteTasks.end())
    {
        delete *task;
        ++task;
    }
}

bool MapGenerator::FinishedGeneration() const
{
    return mGenerationWriteTasks.empty();
}

uint64_t MapGenerator::ContinueGeneration()
{
    const uint64_t endTime = System::processTimeInMilliSecs() + 15;
    std::vector<GenerationWriteTask*>::iterator task = mGenerationWriteTasks.begin();

    while (task != mGenerationWriteTasks.end())
    {
        (*task)->doWork(static_cast<long long>(endTime));
        if ((*task)->hasFinished())
        {
            delete *task;
            task = mGenerationWriteTasks.erase(task);
        }
        else
        {
            ++task;
        }

        if (System::processTimeInMilliSecs() >= endTime)
        {
            break;
        }
    }

    return FinishedGeneration();
}

unsigned int MapGenerator::GetBlockCountOnLayer(unsigned int layer)
{
    return 0;
}
