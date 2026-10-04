#include "game/world/generation/StructureFrequencyAnalyser.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

int countFeaturesInArea(char (*isFeatureChunk)(BiomeSource*, long long, int, int, int),
                        BiomeSource* biomeSource,
                        long long seed,
                        int sizeInChunks);

int countFeaturesInArea(char (*isFeatureChunk)(BiomeSource*, long long, int, int, int),
                        BiomeSource* biomeSource,
                        long long seed,
                        int sizeInChunks)
{
    int halfSizeInChunks = sizeInChunks / 2;
    int featureCount = 0;

    for (int chunkX = -halfSizeInChunks; chunkX <= halfSizeInChunks; ++chunkX)
    {
        for (int chunkZ = -halfSizeInChunks; chunkZ <= halfSizeInChunks; ++chunkZ)
        {
            if (isFeatureChunk(biomeSource, seed, sizeInChunks, chunkX, chunkZ))
                ++featureCount;
        }
    }

    return featureCount;
}

static int _standardTests(void*)
{
    IntCache::CreateNewThreadStorage();

    StructureFrequencyAnalyser analysers[1] = {StructureFrequencyAnalyser(54, false, false)};
    analysers[0].run(1000);
    analysers[0].print();

    IntCache::ReleaseThreadStorage();
    return 0;
}

StructureFrequencyAnalyser::StructureFrequencyAnalyser(int sizeInChunks,
                                                       bool printResults,
                                                       bool printAverages)
    : mSizeInChunks(sizeInChunks), mAreaInChunks(sizeInChunks * sizeInChunks),
      mPrintResults(printResults), mPrintAverages(printAverages)
{
}

StructureFrequencyAnalyser::~StructureFrequencyAnalyser()
{
}

void StructureFrequencyAnalyser::run(unsigned int seedCount)
{
    int widthInBlocks = mSizeInChunks * 16;
    int halfWidthInBlocks = widthInBlocks / 2;
    arrayWithLength<Biome*> biomes(widthInBlocks * widthInBlocks, true);

    for (unsigned int seedIndex = 0; seedIndex < seedCount; ++seedIndex)
    {
        BiomeSource::LayerOverrideSettings layerOverrideSettings;
        layerOverrideSettings.GenerateNewSeed();

        Results results;
        results.seed = layerOverrideSettings.GetSeed();

        BiomeSource biomeSource(results.seed, NULL, NULL, &layerOverrideSettings);
        biomeSource.getBiomeBlock(biomes,
                                  -halfWidthInBlocks,
                                  -halfWidthInBlocks,
                                  widthInBlocks,
                                  widthInBlocks,
                                  false);

        results.woodlandMansions = countFeaturesInArea(WoodlandMansionFeature::fjIsFeatureChunk,
                                                       &biomeSource,
                                                       results.seed,
                                                       mSizeInChunks);
        results.oceanMonuments = countFeaturesInArea(OceanMonumentFeature::fjIsFeatureChunk,
                                                     &biomeSource,
                                                     results.seed,
                                                     mSizeInChunks);
        results.shipwrecks = countFeaturesInArea(ShipwreckFeature::fjIsFeatureChunk,
                                                 &biomeSource,
                                                 results.seed,
                                                 mSizeInChunks);
        mResults.push_back(results);
    }

    delete[] biomes.data;
}

void StructureFrequencyAnalyser::print() const
{
    for (std::vector<Results>::const_iterator result = mResults.begin(); result != mResults.end(); ++result)
    {
    }
}

void StructureFrequencyAnalyser::runTests()
{
    static C4JThreadImpl* testThread = new C4JThreadImpl(_standardTests, NULL, "StructureFrequencyAnalyser", 0);

    if (!testThread->isRunning())
    {
        testThread->Run();
    }
}
