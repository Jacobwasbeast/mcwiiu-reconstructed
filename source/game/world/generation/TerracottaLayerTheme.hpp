#ifndef GAME_WORLD_GENERATION_TERRACOTTALAYERTHEME_HPP
#define GAME_WORLD_GENERATION_TERRACOTTALAYERTHEME_HPP

#include <boost/functional/hash.hpp>

class Random;
class BlockTransaction;

typedef unsigned char (*TerracottaLayerThemeFunction)(int, int);

struct TerracottaLayerThemePattern
{
    int size;
    const unsigned int* values;
    unsigned int patternRows;
    unsigned int patternColumns;
    unsigned int patternLayers;
    unsigned int patternNorth;
    unsigned int patternEast;
    unsigned int patternSouth;
    unsigned int patternWest;
    unsigned int patternClockwise;
    unsigned int patternCounterClockwise;
    unsigned int patternOpposite;
    unsigned int patternPrimaryDirection;
    unsigned int patternSecondaryDirection;
    unsigned int patternTertiaryDirection;
    unsigned int patternQuaternaryDirection;
    unsigned int patternOrientation;
};

class TerracottaLayerTheme
{
public:
    TerracottaLayerTheme(const TerracottaLayerThemePattern& pattern, TerracottaLayerThemeFunction function);

    static unsigned int CONCENTRIC_RINGS(int x, int z);
    static unsigned int OFFSET_RINGS(int x, int z);
    static unsigned int HASHED(int x, int z);
    static unsigned char TARTAN(int x, int z);
    static unsigned int STRIPES(int x, int z);
    static unsigned int UNIFORM(int x, int z);

    virtual void Apply(Random& random, BlockTransaction* transaction, int& width, int& height) const;

private:
    int mSize;
    const unsigned int* mValues;
    unsigned int mPatternRows;
    unsigned int mPatternColumns;
    unsigned int mPatternLayers;
    unsigned int mPatternNorth;
    unsigned int mPatternEast;
    unsigned int mPatternSouth;
    unsigned int mPatternWest;
    unsigned int mPatternClockwise;
    unsigned int mPatternCounterClockwise;
    unsigned int mPatternOpposite;
    unsigned int mPatternPrimaryDirection;
    unsigned int mPatternSecondaryDirection;
    unsigned int mPatternTertiaryDirection;
    unsigned int mPatternQuaternaryDirection;
    unsigned int mPatternOrientation;
    TerracottaLayerThemeFunction mFunction;
};

#endif
