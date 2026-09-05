#include "game/world/biome/TheEndBiomeDecorator.hpp"

#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/EndPodiumFeature.hpp"
#include "game/util/Random.hpp"

TheEndBiomeDecorator::TheEndBiomeDecorator()
    : BiomeDecorator(),
      spikeGen(new SpikeFeature()),
      endPodiumGen(new EndPodiumFeature(false))
{
}

TheEndBiomeDecorator::~TheEndBiomeDecorator()
{
}

void TheEndBiomeDecorator::decorate(Biome* biome, Level* level, Random* random)
{
    decorateOres(level, random);

    static const int spikeData[4][7] =
    {
        { 32, -16, 40, 0, 2, 12, 0 },
        { 16, 16, 28, 28, 2, 15, 0 },
        { -16, 32, 0, 40, 2, 18, 0 },
        { -32, 16, -28, 28, 3, 21, 0 }
    };

    for (int i = 0; i < 4; ++i)
    {
        if (position.x == spikeData[i][0] && position.z == spikeData[i][1])
        {
            SpikeFeature::EndSpike spike(spikeData[i][2], spikeData[i][3],
                                         spikeData[i][4], spikeData[i][5],
                                         spikeData[i][6] != 0);
            spikeGen->setSpike(&spike);
            spikeGen->place(level, *random,
                            BlockPos(spikeData[i][2], spikeData[i][5], spikeData[i][3]));
        }
    }
}

std::vector<SpikeFeature::EndSpike>* TheEndBiomeDecorator::getSpikesForLevel(Level* level)
{
    (void)level;
    static const int spikeData[8][5] =
    {
        { 40, 0, 2, 12, 0 },
        { 28, 28, 2, 15, 0 },
        { 0, 40, 2, 18, 0 },
        { -28, 28, 3, 21, 0 },
        { -40, 0, 3, 24, 0 },
        { -40, -40, 3, 27, 0 },
        { 0, -40, 4, 30, 1 },
        { -32, -28, 4, 33, 1 }
    };

    std::vector<SpikeFeature::EndSpike>* spikes =
        new std::vector<SpikeFeature::EndSpike>(8);
    for (int i = 0; i < 8; ++i)
    {
        (*spikes)[i] = SpikeFeature::EndSpike(
            spikeData[i][0], spikeData[i][1], spikeData[i][2],
            spikeData[i][3], spikeData[i][4] != 0);
    }
    return spikes;
}
