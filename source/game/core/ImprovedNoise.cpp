#include "game/core/ImprovedNoise.hpp"

#include "game/util/Random.hpp"

static double gradX[16] =
{
    1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 0.0
};

static double gradY[16] =
{
    1.0, 1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0
};

static double gradZ[16] =
{
    0.0, 0.0, 0.0, 0.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 0.0, 1.0, 0.0, -1.0
};

static double grad2X[16] =
{
    1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 0.0
};

static double grad2Z[16] =
{
    0.0, 0.0, 0.0, 0.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 0.0, 1.0, 0.0, -1.0
};

struct ImprovedNoiseYSample
{
    double frac;
    double fade;
    int index;
};

static int floorDouble(double value)
{
    int truncated = (int)value;

    if (value < (double)truncated)
    {
        return truncated - 1;
    }

    return truncated;
}

static double fade(double t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

static double lerp(double t, double from, double to)
{
    return from + t * (to - from);
}

static double grad(int hash, double x, double y, double z)
{
    int i = hash & 15;

    return gradX[i] * x + gradY[i] * y + gradZ[i] * z;
}

static double grad2(int hash, double x, double z)
{
    int i = hash & 15;

    return grad2X[i] * x + grad2Z[i] * z;
}

ImprovedNoise::ImprovedNoise(Random* random)
{
    init(random);
}

ImprovedNoise::~ImprovedNoise()
{
    delete[] p;
}

void ImprovedNoise::init(Random* random)
{
    p = new int[512];

    xo = random->nextDouble() * 256.0;
    yo = random->nextDouble() * 256.0;
    zo = random->nextDouble() * 256.0;

    for (int i = 0; i < 256; ++i)
    {
        p[i] = i;
    }

    for (int i = 0; i < 256; ++i)
    {
        int j = random->nextInt(256 - i) + i;
        int value = p[i];

        p[i] = p[j];
        p[j] = value;
        p[i + 256] = p[i];
    }
}

double ImprovedNoise::noise(double x, double y, double z)
{
    double xin = x + xo;
    double yin = y + yo;
    double zin = z + zo;

    int xFloor = floorDouble(xin);
    int yFloor = floorDouble(yin);
    int zFloor = floorDouble(zin);

    xin -= (double)xFloor;
    yin -= (double)yFloor;
    zin -= (double)zFloor;

    int X = xFloor & 255;
    int Y = yFloor & 255;
    int Z = zFloor & 255;

    double u = fade(xin);
    double v = fade(yin);
    double w = fade(zin);

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    return lerp(w,
                lerp(v,
                     lerp(u, grad(p[AA], xin, yin, zin), grad(p[BA], xin - 1.0, yin, zin)),
                     lerp(u, grad(p[AB], xin, yin - 1.0, zin), grad(p[BB], xin - 1.0, yin - 1.0, zin))),
                lerp(v,
                     lerp(u, grad(p[AA + 1], xin, yin, zin - 1.0), grad(p[BA + 1], xin - 1.0, yin, zin - 1.0)),
                     lerp(u, grad(p[AB + 1], xin, yin - 1.0, zin - 1.0), grad(p[BB + 1], xin - 1.0, yin - 1.0, zin - 1.0))));
}

double ImprovedNoise::getValue(double x, double y)
{
    return noise(x, y, 0.0);
}

double ImprovedNoise::getValue(double x, double y, double z)
{
    return noise(x, y, z);
}

void ImprovedNoise::add(arrayWithLength<double> values, double x, double y, double z, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale, double noiseScale)
{
    double xin = x + xo;
    double invNoiseScale = 1.0 / noiseScale;
    int index = 0;

    if (ySize == 1)
    {
        for (int xi = 0; xi < xSize; ++xi)
        {
            int xFloor = floorDouble(xin);
            double xFrac = xin - (double)xFloor;
            int X = xFloor & 255;
            double xFade = fade(xFrac);
            double zin = z + zo;

            int A = p[p[X]];
            int B = p[p[X + 1]];

            for (int zi = 0; zi < zSize; ++zi)
            {
                int zFloor = floorDouble(zin);
                double zFrac = zin - (double)zFloor;
                int Z = zFloor & 255;
                double zFade = fade(zFrac);

                int AA = A + Z;
                int BA = B + Z;

                double lerpZ0 = lerp(xFade, grad2(p[AA], xFrac, zFrac), grad(p[BA], xFrac - 1.0, 0.0, zFrac));
                double lerpZ1 = lerp(xFade, grad(p[AA + 1], xFrac, 0.0, zFrac - 1.0), grad(p[BA + 1], xFrac - 1.0, 0.0, zFrac - 1.0));

                values.data[index++] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;

                zin += zScale;
            }

            xin += xScale;
        }
    }
    else
    {
        ImprovedNoiseYSample ySamples[64];

        int lastY = -1;
        bool fillYSamples = true;

        double lerpY0Z0 = 0.0;
        double lerpY1Z0 = 0.0;
        double lerpY0Z1 = 0.0;
        double lerpY1Z1 = 0.0;

        for (int xi = 0; xi < xSize; ++xi)
        {
            int xFloor = floorDouble(xin);
            double xFrac = xin - (double)xFloor;
            int X = xFloor & 255;
            double xFade = fade(xFrac);
            double zin = z + zo;

            for (int zi = 0; zi < zSize; ++zi)
            {
                int zFloor = floorDouble(zin);
                double zFrac = zin - (double)zFloor;
                int Z = zFloor & 255;
                double zFade = fade(zFrac);
                double yin = y + yo;

                for (int yi = 0; yi < ySize; ++yi)
                {
                    double yFrac;
                    double yFade;
                    int Y;

                    if (fillYSamples)
                    {
                        int yFloor = floorDouble(yin);

                        yFrac = yin - (double)yFloor;
                        Y = yFloor & 255;
                        yFade = fade(yFrac);

                        ySamples[yi].frac = yFrac;
                        ySamples[yi].fade = yFade;
                        ySamples[yi].index = Y;

                        yin += yScale;
                    }
                    else
                    {
                        yFrac = ySamples[yi].frac;
                        yFade = ySamples[yi].fade;
                        Y = ySamples[yi].index;
                    }

                    if (yi == 0 || Y != lastY)
                    {
                        int A = p[X] + Y;
                        int AA = p[A] + Z;
                        int AB = p[A + 1] + Z;
                        int B = p[X + 1] + Y;
                        int BA = p[B] + Z;
                        int BB = p[B + 1] + Z;

                        lerpY0Z0 = lerp(xFade, grad(p[AA], xFrac, yFrac, zFrac), grad(p[BA], xFrac - 1.0, yFrac, zFrac));
                        lerpY1Z0 = lerp(xFade, grad(p[AB], xFrac, yFrac - 1.0, zFrac), grad(p[BB], xFrac - 1.0, yFrac - 1.0, zFrac));
                        lerpY0Z1 = lerp(xFade, grad(p[AA + 1], xFrac, yFrac, zFrac - 1.0), grad(p[BA + 1], xFrac - 1.0, yFrac, zFrac - 1.0));
                        lerpY1Z1 = lerp(xFade, grad(p[AB + 1], xFrac, yFrac - 1.0, zFrac - 1.0), grad(p[BB + 1], xFrac - 1.0, yFrac - 1.0, zFrac - 1.0));

                        lastY = Y;
                    }

                    double lerpZ0 = lerp(yFade, lerpY0Z0, lerpY1Z0);
                    double lerpZ1 = lerp(yFade, lerpY0Z1, lerpY1Z1);

                    values.data[index++] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;
                }

                fillYSamples = false;

                zin += zScale;
            }

            xin += xScale;
        }
    }
}
