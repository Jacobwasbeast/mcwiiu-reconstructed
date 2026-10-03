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

static inline double lerp(double t, double from, double to)
{
    return from + t * (to - from);
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

    int xFloor = (int)xin;

    if (xin < (double)xFloor)
    {
        xFloor--;
    }

    int yFloor = (int)yin;

    if (yin < (double)yFloor)
    {
        yFloor--;
    }

    int zFloor = (int)zin;

    if (zin < (double)zFloor)
    {
        zFloor--;
    }

    xin -= (double)xFloor;
    yin -= (double)yFloor;
    zin -= (double)zFloor;

    int X = xFloor & 255;
    int Y = yFloor & 255;
    int Z = zFloor & 255;

    double u = xin * xin * xin * (xin * (xin * 6.0 - 15.0) + 10.0);
    double v = yin * yin * yin * (yin * (yin * 6.0 - 15.0) + 10.0);
    double w = zin * zin * zin * (zin * (zin * 6.0 - 15.0) + 10.0);

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    int g000Hash = p[AA] & 15;
    double g000 = gradX[g000Hash] * xin + gradY[g000Hash] * yin + gradZ[g000Hash] * zin;
    int g100Hash = p[BA] & 15;
    double g100 = gradX[g100Hash] * (xin - 1.0) + gradY[g100Hash] * yin + gradZ[g100Hash] * zin;
    int g010Hash = p[AB] & 15;
    double g010 = gradX[g010Hash] * xin + gradY[g010Hash] * (yin - 1.0) + gradZ[g010Hash] * zin;
    int g110Hash = p[BB] & 15;
    double g110 = gradX[g110Hash] * (xin - 1.0) + gradY[g110Hash] * (yin - 1.0) + gradZ[g110Hash] * zin;
    int g001Hash = p[AA + 1] & 15;
    double g001 = gradX[g001Hash] * xin + gradY[g001Hash] * yin + gradZ[g001Hash] * (zin - 1.0);
    int g101Hash = p[BA + 1] & 15;
    double g101 = gradX[g101Hash] * (xin - 1.0) + gradY[g101Hash] * yin + gradZ[g101Hash] * (zin - 1.0);
    int g011Hash = p[AB + 1] & 15;
    double g011 = gradX[g011Hash] * xin + gradY[g011Hash] * (yin - 1.0) + gradZ[g011Hash] * (zin - 1.0);
    int g111Hash = p[BB + 1] & 15;
    double g111 = gradX[g111Hash] * (xin - 1.0) + gradY[g111Hash] * (yin - 1.0) + gradZ[g111Hash] * (zin - 1.0);

    return lerp(w,
                lerp(v, lerp(u, g000, g100), lerp(u, g010, g110)),
                lerp(v, lerp(u, g001, g101), lerp(u, g011, g111)));
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
            int xFloor = (int)xin;

            if (xin < (double)xFloor)
            {
                xFloor--;
            }

            int X = xFloor & 255;
            double xFade = xin * xin * xin * (xin * (xin * 6.0 - 15.0) + 10.0);
            double zin = z + zo;

            int A = p[p[X]];
            int B = p[p[X + 1]];

            for (int zi = 0; zi < zSize; ++zi)
            {
                int zFloor = (int)zin;

                if (zin < (double)zFloor)
                {
                    zFloor--;
                }

                int Z = zFloor & 255;
                double zFade = zin * zin * zin * (zin * (zin * 6.0 - 15.0) + 10.0);

                int AA = A + Z;
                int BA = B + Z;

                int g00Hash = p[AA] & 15;
                double g00 = grad2X[g00Hash] * xin + grad2Z[g00Hash] * zin;
                int g10Hash = p[BA] & 15;
                double g10 = gradX[g10Hash] * (xin - 1.0) + gradY[g10Hash] * 0.0 + gradZ[g10Hash] * zin;
                int g01Hash = p[AA + 1] & 15;
                double g01 = gradX[g01Hash] * xin + gradY[g01Hash] * 0.0 + gradZ[g01Hash] * (zin - 1.0);
                int g11Hash = p[BA + 1] & 15;
                double g11 = gradX[g11Hash] * (xin - 1.0) + gradY[g11Hash] * 0.0 + gradZ[g11Hash] * (zin - 1.0);

                double lerpZ0 = lerp(xFade, g00, g10);
                double lerpZ1 = lerp(xFade, g01, g11);

                int outIndex = index++;
                values.data[outIndex] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;

                zin = zin + (double)zFloor + zScale;
            }

            xin = xin + (double)xFloor + xScale;
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
            int xFloor = (int)xin;

            if (xin < (double)xFloor)
            {
                xFloor--;
            }

            xin -= (double)xFloor;
            int X = xFloor & 255;
            double xFade = xin * xin * xin * (xin * (xin * 6.0 - 15.0) + 10.0);
            double zin = z + zo;

            for (int zi = 0; zi < zSize; ++zi)
            {
                int zFloor = (int)zin;

                if (zin < (double)zFloor)
                {
                    zFloor--;
                }

                zin -= (double)zFloor;
                int Z = zFloor & 255;
                double zFade = zin * zin * zin * (zin * (zin * 6.0 - 15.0) + 10.0);

                if (fillYSamples)
                {
                    fillYSamples = false;

                    double yin = y + yo;

                    for (int yi = 0; yi < ySize; ++yi)
                    {
                        int yFloor = (int)yin;

                        if (yin < (double)yFloor)
                        {
                            yFloor--;
                        }

                        yin -= (double)yFloor;
                        int Y = yFloor & 255;
                        double yFrac = yin;
                        double yFade = yFrac * yFrac * yFrac * (yFrac * (yFrac * 6.0 - 15.0) + 10.0);

                        ySamples[yi].frac = yFrac;
                        ySamples[yi].fade = yFade;
                        ySamples[yi].index = Y;

                        if (yi == 0 || Y != lastY)
                        {
                            int A = p[X] + Y;
                            int AA = p[A] + Z;
                            int AB = p[A + 1] + Z;
                            int B = p[X + 1] + Y;
                            int BA = p[B] + Z;
                            int BB = p[B + 1] + Z;

                            int g000Hash = p[AA] & 15;
                            double g000 = gradX[g000Hash] * xin + gradY[g000Hash] * yFrac + gradZ[g000Hash] * zin;
                            int g100Hash = p[BA] & 15;
                            double g100 = gradX[g100Hash] * (xin - 1.0) + gradY[g100Hash] * yFrac + gradZ[g100Hash] * zin;
                            int g010Hash = p[AB] & 15;
                            double g010 = gradX[g010Hash] * xin + gradY[g010Hash] * (yFrac - 1.0) + gradZ[g010Hash] * zin;
                            int g110Hash = p[BB] & 15;
                            double g110 = gradX[g110Hash] * (xin - 1.0) + gradY[g110Hash] * (yFrac - 1.0) + gradZ[g110Hash] * zin;
                            int g001Hash = p[AA + 1] & 15;
                            double g001 = gradX[g001Hash] * xin + gradY[g001Hash] * yFrac + gradZ[g001Hash] * (zin - 1.0);
                            int g101Hash = p[BA + 1] & 15;
                            double g101 = gradX[g101Hash] * (xin - 1.0) + gradY[g101Hash] * yFrac + gradZ[g101Hash] * (zin - 1.0);
                            int g011Hash = p[AB + 1] & 15;
                            double g011 = gradX[g011Hash] * xin + gradY[g011Hash] * (yFrac - 1.0) + gradZ[g011Hash] * (zin - 1.0);
                            int g111Hash = p[BB + 1] & 15;
                            double g111 = gradX[g111Hash] * (xin - 1.0) + gradY[g111Hash] * (yFrac - 1.0) + gradZ[g111Hash] * (zin - 1.0);

                            lerpY0Z0 = lerp(xFade, g000, g100);
                            lerpY1Z0 = lerp(xFade, g010, g110);
                            lerpY0Z1 = lerp(xFade, g001, g101);
                            lerpY1Z1 = lerp(xFade, g011, g111);

                            lastY = Y;
                        }

                        double lerpZ0 = lerp(yFade, lerpY0Z0, lerpY1Z0);
                        double lerpZ1 = lerp(yFade, lerpY0Z1, lerpY1Z1);

                        int outIndex = index++;
                        values.data[outIndex] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;

                        yin = yin + (double)yFloor + yScale;
                    }
                }
                else
                {
                    for (int yi = 0; yi < ySize; ++yi)
                    {
                        double yFrac = ySamples[yi].frac;
                        double yFade = ySamples[yi].fade;
                        int Y = ySamples[yi].index;

                        if (yi == 0 || Y != lastY)
                        {
                            int A = p[X] + Y;
                            int AA = p[A] + Z;
                            int AB = p[A + 1] + Z;
                            int B = p[X + 1] + Y;
                            int BA = p[B] + Z;
                            int BB = p[B + 1] + Z;

                            int g000Hash = p[AA] & 15;
                            double g000 = gradX[g000Hash] * xin + gradY[g000Hash] * yFrac + gradZ[g000Hash] * zin;
                            int g100Hash = p[BA] & 15;
                            double g100 = gradX[g100Hash] * (xin - 1.0) + gradY[g100Hash] * yFrac + gradZ[g100Hash] * zin;
                            int g010Hash = p[AB] & 15;
                            double g010 = gradX[g010Hash] * xin + gradY[g010Hash] * (yFrac - 1.0) + gradZ[g010Hash] * zin;
                            int g110Hash = p[BB] & 15;
                            double g110 = gradX[g110Hash] * (xin - 1.0) + gradY[g110Hash] * (yFrac - 1.0) + gradZ[g110Hash] * zin;
                            int g001Hash = p[AA + 1] & 15;
                            double g001 = gradX[g001Hash] * xin + gradY[g001Hash] * yFrac + gradZ[g001Hash] * (zin - 1.0);
                            int g101Hash = p[BA + 1] & 15;
                            double g101 = gradX[g101Hash] * (xin - 1.0) + gradY[g101Hash] * yFrac + gradZ[g101Hash] * (zin - 1.0);
                            int g011Hash = p[AB + 1] & 15;
                            double g011 = gradX[g011Hash] * xin + gradY[g011Hash] * (yFrac - 1.0) + gradZ[g011Hash] * (zin - 1.0);
                            int g111Hash = p[BB + 1] & 15;
                            double g111 = gradX[g111Hash] * (xin - 1.0) + gradY[g111Hash] * (yFrac - 1.0) + gradZ[g111Hash] * (zin - 1.0);

                            lerpY0Z0 = lerp(xFade, g000, g100);
                            lerpY1Z0 = lerp(xFade, g010, g110);
                            lerpY0Z1 = lerp(xFade, g001, g101);
                            lerpY1Z1 = lerp(xFade, g011, g111);

                            lastY = Y;
                        }

                        double lerpZ0 = lerp(yFade, lerpY0Z0, lerpY1Z0);
                        double lerpZ1 = lerp(yFade, lerpY0Z1, lerpY1Z1);

                        int outIndex = index++;
                        values.data[outIndex] += lerp(zFade, lerpZ0, lerpZ1) * invNoiseScale;
                    }
                }

                zin = zin + (double)zFloor + zScale;
            }

            xin = xin + (double)xFloor + xScale;
        }
    }
}
