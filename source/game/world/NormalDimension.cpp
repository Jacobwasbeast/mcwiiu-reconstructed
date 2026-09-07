#include "game/world/NormalDimension.hpp"

#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"

NormalDimension::NormalDimension()
    : Dimension()
{
    mXZSize = 63;
}

NormalDimension::~NormalDimension()
{
}

const DimensionType* NormalDimension::getType() const
{
    return DimensionType::OVERWORLD;
}

bool NormalDimension::canUnloadChunk(int x, int z)
{
    return !mLevel->isSpawnChunk(x, z);
}
