#include "game/core/SimplexNoise.hpp"

#include "game/util/Random.hpp"

#include <cmath>

static int grad3[12][3] =
{
    {  1,  1,  0 },
    { -1,  1,  0 },
    {  1, -1,  0 },
    { -1, -1,  0 },
    {  1,  0,  1 },
    { -1,  0,  1 },
    {  1,  0, -1 },
    { -1,  0, -1 },
    {  0,  1,  1 },
    {  0, -1,  1 },
    {  0,  1, -1 },
    {  0, -1, -1 }
};

static const double SQRT_3 = std::sqrt(3.0);
static const double F2 = 0.5 * (SQRT_3 - 1.0);
static const double G2 = (3.0 - SQRT_3) / 6.0;

SimplexNoise::SimplexNoise(Random* random)
{
    init(random);
}

SimplexNoise::~SimplexNoise()
{
    delete[] p;
}

void SimplexNoise::init(Random* random)
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

int SimplexNoise::fastfloor(double value)
{
    return value > 0.0 ? (int)value : (int)value - 1;
}

double SimplexNoise::dot(int* grad, double x, double y)
{
    return grad[0] * x + grad[1] * y;
}

double SimplexNoise::getValue(double x, double y)
{
    double s = (x + y) * F2;
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    double t = (i + j) * G2;
    double X0 = i - t;
    double Y0 = j - t;
    double x0 = x - X0;
    double y0 = y - Y0;

    int i1;
    int j1;

    if (x0 > y0)
    {
        i1 = 1;
        j1 = 0;
    }
    else
    {
        i1 = 0;
        j1 = 1;
    }

    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 255;
    int jj = j & 255;
    int gi0 = p[ii + p[jj]] % 12;
    int gi1 = p[ii + i1 + p[jj + j1]] % 12;
    int gi2 = p[ii + 1 + p[jj + 1]] % 12;

    double n0;
    double n1;
    double n2;

    double t0 = 0.5 - x0 * x0 - y0 * y0;

    if (t0 < 0.0)
    {
        n0 = 0.0;
    }
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;

    if (t1 < 0.0)
    {
        n1 = 0.0;
    }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }

    double t2 = 0.5 - x2 * x2 - y2 * y2;

    if (t2 < 0.0)
    {
        n2 = 0.0;
    }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }

    return 70.0 * (n0 + n1 + n2);
}

void SimplexNoise::add(arrayWithLength<double> values, double x, double y, int xSize, int ySize, double xScale, double yScale, double noiseScale)
{
    int index = 0;

    for (int j = 0; j < ySize; ++j)
    {
        double yin = (y + j) * yScale + yo;

        for (int i = 0; i < xSize; ++i)
        {
            double xin = (x + i) * xScale + xo;

            values.data[index++] += getValue(xin, yin) * noiseScale;
        }
    }
}
