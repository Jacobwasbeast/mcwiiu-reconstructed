#ifndef GAME_UTIL_RANDOMVALUEBOUNDS_HPP
#define GAME_UTIL_RANDOMVALUEBOUNDS_HPP

class RandomValueBounds
{
public:
    float min;
    float max;

    RandomValueBounds();
    RandomValueBounds(int min, int max);
    RandomValueBounds(double min, double max);
    RandomValueBounds(int min, double max);
    RandomValueBounds(int value);
    RandomValueBounds(float min, float max);
    RandomValueBounds(float value);
    RandomValueBounds(double value);
};

#endif
