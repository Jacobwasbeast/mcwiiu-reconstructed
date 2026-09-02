#include "game/core/PerlinSimplexNoise.hpp"

#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"

void MemSect(int section);

PerlinSimplexNoise::PerlinSimplexNoise(long long seed, int levels)
{
    Random random(seed);

    init(&random, levels);
}

PerlinSimplexNoise::PerlinSimplexNoise(Random* random, int levels)
{
    init(random, levels);
}

PerlinSimplexNoise::~PerlinSimplexNoise()
{
    for (int i = 0; i < levels; i++)
    {
        delete noiseLevels[i];
    }

    delete[] noiseLevels;
}

void PerlinSimplexNoise::init(Random* random, int levels)
{
    MemSect(3);

    this->levels = levels;
    noiseLevels = new SimplexNoise*[levels];

    for (int i = 0; i < levels; i++)
    {
        noiseLevels[i] = new SimplexNoise(random);
    }

    MemSect(0);
}

double PerlinSimplexNoise::getValue(double x, double y)
{
    double value = 0.0;
    double frequency = 1.0;

    for (int i = 0; i < levels; i++)
    {
        value += noiseLevels[i]->getValue(x * frequency, y * frequency) / frequency;

        frequency /= 2.0;
    }

    return value;
}

arrayWithLength<double> PerlinSimplexNoise::getRegion(arrayWithLength<double> values, double x, double y, int xSize, int ySize, double xScale, double yScale, double frequencyScale)
{
    return getRegion(values, x, y, xSize, ySize, xScale, yScale, frequencyScale, 0.5);
}

arrayWithLength<double> PerlinSimplexNoise::getRegion(arrayWithLength<double> values, double x, double y, int xSize, int ySize, double xScale, double yScale, double frequencyScale, double amplitudeScale)
{
    xScale /= 1.5;
    yScale /= 1.5;

    if (values.data != NULL && (int)values.length >= xSize * ySize)
    {
        for (unsigned int i = 0; i < values.length; i++)
        {
            values.data[i] = 0.0;
        }
    }
    else
    {
        if (values.data != NULL)
        {
            delete[] values.data;
        }

        values = arrayWithLength<double>(xSize * ySize, true);
    }

    double frequency = 1.0;
    double amplitude = 1.0;

    for (int i = 0; i < levels; i++)
    {
        noiseLevels[i]->add(values, x, y, xSize, ySize, xScale * frequency, yScale * frequency, 0.55 / amplitude);

        amplitude *= amplitudeScale;
        frequency *= frequencyScale;
    }

    return values;
}
