#include "game/world/generation/FunctionPatchesLayerTheme.hpp"

#include "game/core/DyeColor.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/WoolCarpetBlock.hpp"

FunctionPatchesLayerTheme::FunctionPatchesLayerTheme(unsigned int seed, double noiseScale, BlockFunction blockFunction)
    : RandomPatchesLayerTheme(seed, noiseScale)
    , mBlockFunction(blockFunction)
{
}

const BlockState* FunctionPatchesLayerTheme::GRASS_PATCHES(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise < 0.3)
    {
        return NULL;
    }

    if (heightAboveSurface == ABOVE_SURFACE(1) || heightAboveSurface == ABOVE_SURFACE(2))
    {
        DoublePlantBlock::Half* plantHalf = heightAboveSurface == ABOVE_SURFACE(2)
            ? DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER]
            : DoublePlantBlock::HALVES[DoublePlantBlock::Half::LOWER];
        const BlockState* state = Blocks::double_plant->defaultBlockState();
        state = state->setValue(DoublePlantBlock::VARIANT, DoublePlantBlock::VARIANTS[2]);
        return state->setValue(DoublePlantBlock::HALF, plantHalf);
    }

    if (heightAboveSurface == 0)
    {
        const BlockState* state = Blocks::dirt->defaultBlockState();
        return state->setValue(DirtBlock::VARIANT, DirtBlock::Variant::PODZOL);
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::HELLFIRE_PATCHES(Random&, float noise, int heightAboveSurface, bool isAir)
{
    if (noise >= 0.5)
    {
        if (!isAir && heightAboveSurface == 0)
        {
            return Blocks::netherrack->defaultBlockState();
        }

        if (isAir && heightAboveSurface == ABOVE_SURFACE(1))
        {
            return Blocks::fire->defaultBlockState();
        }
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::SNOWLAYER_PATCHES(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise > 0.4 && heightAboveSurface == ABOVE_SURFACE(1))
    {
        int layers = static_cast<int>((5.0 * (static_cast<double>(noise) - 0.4)) / 0.6) + 1;
        const BlockState* state = Blocks::snow_layer->defaultBlockState();
        return state->setValue(SnowLayerBlock::LAYERS, layers);
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::TNT_TRAPS(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise >= 0.6)
    {
        int aboveSurface = ABOVE_SURFACE(1);
        if (heightAboveSurface >= aboveSurface)
        {
            if (heightAboveSurface == aboveSurface)
            {
                return Blocks::stone_pressure_plate->defaultBlockState();
            }

            return NULL;
        }

        if (heightAboveSurface == 0)
        {
            return Blocks::gravel->defaultBlockState();
        }

        if (heightAboveSurface == BELOW_SURFACE(1))
        {
            return Blocks::tnt->defaultBlockState();
        }

        return Blocks::air->defaultBlockState();
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::CARPET_TRAP(Random&, float noise, int heightAboveSurface, bool)
{
    if (noise <= 0.11f)
    {
        if (heightAboveSurface == ABOVE_SURFACE(1))
        {
            return Blocks::carpet->getBlockState(14);
        }

        if (noise < -0.43f && heightAboveSurface < 1)
        {
            return Blocks::air->defaultBlockState();
        }
    }

    return NULL;
}

const BlockState* FunctionPatchesLayerTheme::BlockAt(Random& random, float noise, int heightAboveSurface, bool isAir) const
{
    return mBlockFunction(random, noise, heightAboveSurface, isAir);
}
